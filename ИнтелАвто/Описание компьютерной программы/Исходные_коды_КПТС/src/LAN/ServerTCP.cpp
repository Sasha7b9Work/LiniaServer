// 2025/02/20 11:44:48 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "LAN/ServerTCP.h"
#include "Settings/Settings.h"
#include "Hardware/Timer.h"
#include "Device/IT6523.h"
#include <lwip/tcp.h>
#include <cstring>

namespace ServerTCP
{
    static tcp_pcb *pcbServer = nullptr;

    enum State
    {
        S_NOT_CONNECTED = 0,
        S_CONNECTED,
        S_RECEIVED,
        S_CLOSING,
    };

    struct Server
    {
        State    state;     // connection status
        tcp_pcb *pcb;       // pointer on the current tcp_pcb
        pbuf    *p_tx;      // pointer on pbuf to be transmitted
    };

    static Server *server = nullptr;

    static void (*SocketFuncReciever)(pchar buffer, uint length) = nullptr;

    static void Send(tcp_pcb *, Server *);
    static void CloseConnection(tcp_pcb *, Server *);

    static err_t CallbackOnConnect(void *, tcp_pcb *, err_t);
    static err_t CallbackRecv(void *, tcp_pcb *, pbuf *, err_t);
    static err_t CallbackPoll(void *, tcp_pcb *);
    static err_t CallbackSent(void *, tcp_pcb *, uint16);
    static void CallbackError(void *, err_t);

    static uint time_last_callback_ETH = 0;
}


void ServerTCP::Init(void (*funcReceive)(pchar buffer, uint length))
{
    if (SocketFuncReciever)                             // Идёт подключение к серверу
    {
        return;
    }

    SocketFuncReciever = funcReceive;

    // Создаём клиента для подключения к серверу блока питания

    tcp_pcb *pcb = tcp_new();

    if (pcb != nullptr)
    {
        ip_addr_t ipaddr;

        tcp_err(pcb, CallbackError);

        IP4_ADDR(&ipaddr, gset.ipIT6523[0], gset.ipIT6523[1], gset.ipIT6523[2], gset.ipIT6523[3]);

        tcp_connect(pcb, &ipaddr, gset.portIT6523, CallbackOnConnect);
    }
    else
    {
        SocketFuncReciever = nullptr;
    }
}


err_t ServerTCP::CallbackOnConnect(void * /*arg*/, tcp_pcb *tpcb, err_t err)
{
    if (err == ERR_OK)
    {
        pcbServer = tpcb;

        /* allocate structure es to maintain tcp connection information */
        server = (Server *)mem_malloc(sizeof(Server));

        if (server != nullptr)
        {
            server->state = S_CONNECTED;
            server->pcb = tpcb;
            server->p_tx = nullptr;

            /* pass newly allocated es structure as argument to tpcb */
            tcp_arg(tpcb, server);

            /* initialize LwIP tcp_recv callback function */
            tcp_recv(tpcb, CallbackRecv);

            /* initialize LwIP tcp_sent callback function */
            tcp_sent(tpcb, CallbackSent);

            /* initialize LwIP tcp_poll callback function */
            tcp_poll(tpcb, CallbackPoll, 1);

            /* send data */
            Send(tpcb, server);

            return ERR_OK;
        }
        else
        {
            /* close connection */
            CloseConnection(tpcb, server);

            /* return memory allocation error */
            return ERR_MEM;
        }
    }
    else
    {
        /* close connection */
        CloseConnection(tpcb, server);
    }
    return err;
}


err_t ServerTCP::CallbackRecv(void *arg, tcp_pcb *tpcb, pbuf *p, err_t err)
{
    Server *ss = (Server *)arg;

    if (p == nullptr)
    {
        // remote host closed connection
        ss->state = S_CLOSING;
        if (ss->p_tx == nullptr)
        {
            // we're done sending, close it
            CloseConnection(tpcb, ss);
        }
        else
        {
            // we're not done yet
            tcp_sent(tpcb, CallbackSent);
        }

        err = ERR_OK;
    }
    else if (err != ERR_OK)
    {
        // cleanup, for unkown reason
        if (p != nullptr) //-V547
        {
            ss->p_tx = nullptr;
            pbuf_free(p);
        }
    }
    else if (ss->state == S_RECEIVED || ss->state == S_CONNECTED)
    {
        ss->state = S_RECEIVED;

        // read some more data
        if (ss->p_tx == nullptr)
        {
            //ss->p = _p;
            tcp_sent(tpcb, CallbackSent);
            //Send(_tpcb, ss);
            if (SocketFuncReciever)
            {
                SocketFuncReciever((char *)p->payload, p->len);
            }

            u8_t freed = 0;
            do
            {
                // try hard to free pbuf 
                freed = pbuf_free(p);
            } while (freed == 0);

        }
        else
        {
            pbuf *ptr;
            // chain pbufs to the end of what we recv'ed previously
            ptr = ss->p_tx;
            pbuf_chain(ptr, p);
        }

        err = ERR_OK; //-V1048
    }
    else if (ss->state == S_CLOSING)
    {
        // odd case, remote side closing twice, trash data
        tcp_recved(tpcb, p->tot_len);
        ss->p_tx = nullptr;
        pbuf_free(p);

        err = ERR_OK; //-V1048

        CloseConnection(tpcb, ss);
    }
    else
    {
        // unknown ss->state, trash data
        tcp_recved(tpcb, p->tot_len);
        ss->p_tx = nullptr;
        pbuf_free(p);

        err = ERR_OK; //-V1048
    }

    return err;
}


void ServerTCP::Send(tcp_pcb *tpcb, Server *ss)
{
    err_t wr_err = ERR_OK;

    while ((wr_err == ERR_OK) &&
        (ss->p_tx != nullptr) &&
        (ss->p_tx->len <= tcp_sndbuf(tpcb)))
    {
        pbuf *ptr = ss->p_tx;
        // enqueue data for transmittion
        wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
        tcp_output(tpcb);
        if (wr_err == ERR_OK)
        {
            u16_t plen = ptr->len;
            // continue with new pbuf in chain (if any) 
            ss->p_tx = ptr->next;
            if (ss->p_tx != nullptr)
            {
                // new reference!
                pbuf_ref(ss->p_tx);
            }

            u8_t freed;

            // chop first pbuf from chain
            do
            {
                // try hard to free pbuf 
                freed = pbuf_free(ptr);
            } while (freed == 0);
            // we can read more data now
            tcp_recved(tpcb, plen);
        }
        else if (wr_err == ERR_MEM)
        {
            // we are low on memory, try later / harder, defer to poll
            ss->p_tx = ptr; //-V1048
        }
        else
        {
        }
    }
}


err_t ServerTCP::CallbackPoll(void *arg, tcp_pcb *tpcb)
{
    Server *es = (Server *)arg;

    err_t err = ERR_OK;

    if (es != nullptr)
    {
        if (es->p_tx != nullptr)
        {
            /* there is a remaining pbuf (chain) , try to send data */
            Send(tpcb, es);
        }
        else
        {
            /* no remaining pbuf (chain)  */
            if (es->state == S_CLOSING)
            {
                /* close tcp connection */
                CloseConnection(tpcb, es);
            }
        }
    }
    else
    {
        /* nothing to be done */
        tcp_abort(tpcb);

        err = ERR_ABRT;
    }

    return err;
}


err_t ServerTCP::CallbackSent(void *arg, tcp_pcb *tpcb, u16_t /*len*/)
{
    Server *es = (Server *)arg;

    if (es->p_tx != nullptr)
    {
        /* still got pbufs to send */
        Send(tpcb, es);
    }
    else
    {
        if (es->state == S_CLOSING)
        {
            CloseConnection(tpcb, es);
        }
    }

    return ERR_OK;
}


void ServerTCP::CloseConnection(tcp_pcb *tpcb, Server *es)
{
    /* remove callbacks */
    tcp_recv(tpcb, nullptr);
    tcp_sent(tpcb, nullptr);
    tcp_poll(tpcb, nullptr, 0);

    if (es != nullptr)
    {
        mem_free(es);
    }

    pcbServer = nullptr;

    SocketFuncReciever = nullptr;

    /* close tcp connection */
    tcp_close(tpcb);
}


void ServerTCP::SendBuffer(pchar buffer, uint length)
{
    if (pcbServer)
    {
        pbuf *tcpBuffer = pbuf_alloc(PBUF_RAW, (uint16)length, PBUF_POOL);
        tcpBuffer->flags = 1;
        pbuf_take(tcpBuffer, buffer, (uint16)length);
        Server *ss = (Server *)mem_malloc(sizeof(Server));
        ss->p_tx = tcpBuffer;
        Send(pcbServer, ss);
        mem_free(ss);
    }
}


void ServerTCP::SendString(pchar buffer)
{
    SendBuffer(buffer, std::strlen(buffer));
}


bool ServerTCP::IsConnected()
{
    if (TIME_MS - time_last_callback_ETH > 5000)
    {
        CloseConnection(pcbServer, server);
    }

    static uint time_send = 0;

    if (TIME_MS - time_last_callback_ETH > 3000)
    {
        if (TIME_MS - time_send > 100)
        {
            time_send = TIME_MS;
            IT6523::SendCommand("SYSTEM:ERROR?");
        }
    }

    return pcbServer != nullptr;
}


void ServerTCP::CallbackError(void *arg, err_t)
{
    Server *ss = (Server *)arg;

    CloseConnection(pcbServer, ss);
}


void ServerTCP::CallbackOnRxETH()
{
    time_last_callback_ETH = TIME_MS;
}
