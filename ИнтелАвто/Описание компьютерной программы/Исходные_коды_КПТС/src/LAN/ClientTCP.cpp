// 2025/02/20 11:40:21 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "LAN/ClientTCP.h"
#include <lwip/tcp.h>
#include <cstring>
#include <stdarg.h>
#include "SCPI/SCPI.h"
#include "Settings/Settings.h"
#include "LAN/LAN.h"


namespace ClientTCP
{
    static tcp_pcb *pcbClient = nullptr;     // 0, если клиент не приконнекчен. Сюда коннектится новый пользователь

    enum State
    {
        S_ACCEPTED,
        S_RECIEVED,
        S_CLOSING
    };

    struct Client
    {
        pbuf *p_tx;
        State state;
    };

    static void(*SocketFuncReciever)(pchar buffer, uint length) = nullptr;     // this function will be called when a message is recieved from any client

    static void CloseConnection(tcp_pcb *, Client *);
    static void Send(tcp_pcb *, Client *);

    static err_t CallbackAccept(void *, tcp_pcb *, err_t);
    static err_t CallbackSent(void *, tcp_pcb *, uint16);
    static err_t CallbackRecv(void *, tcp_pcb *, pbuf *, err_t);
    static void CallbackError(void *, err_t);
    static err_t CallbackPoll(void *, tcp_pcb *);
}


void ClientTCP::Init(void (*funcRecieve)(pchar buffer, uint length))
{
    {
        // Создаём сервер для подключения пользователя

        tcp_pcb *pcb = tcp_new();
        if (pcb != nullptr)
        {
            err_t err = tcp_bind(pcb, IP_ADDR_ANY, gset.portSCPI);
            if (err == ERR_OK)
            {
                pcb = tcp_listen(pcb);
                SocketFuncReciever = funcRecieve;
                tcp_accept(pcb, CallbackAccept);
            }
            else
            {
                // abort? output diagnostic?
            }
        }
        else
        {
            // abort? output diagonstic?
        }

        pcbClient = nullptr;
    }
}


bool ClientTCP::IsConnected()
{
    return pcbClient != nullptr;
}


void ClientTCP::CloseConnection(tcp_pcb *tpcb, Client *ss)
{
    tcp_arg(tpcb, nullptr);
    tcp_sent(tpcb, nullptr);
    tcp_recv(tpcb, nullptr);
    tcp_err(tpcb, nullptr);
    tcp_poll(tpcb, nullptr, 0);

    pcbClient = nullptr;

    if (ss)
    {
        mem_free(ss);
    }
    tcp_close(tpcb);
}


void ClientTCP::Send(tcp_pcb *_tpcb, Client *_ss)
{
    err_t wr_err = ERR_OK;

    while ((wr_err == ERR_OK) &&
        (_ss->p_tx != nullptr) &&
        (_ss->p_tx->len <= tcp_sndbuf(_tpcb)))
    {
        pbuf *ptr = _ss->p_tx;
        // enqueue data for transmittion
        wr_err = tcp_write(_tpcb, ptr->payload, ptr->len, 1);
        tcp_output(_tpcb);
        if (wr_err == ERR_OK)
        {
            u16_t plen = ptr->len;
            // continue with new pbuf in chain (if any) 
            _ss->p_tx = ptr->next;
            if (_ss->p_tx != nullptr)
            {
                // new reference!
                pbuf_ref(_ss->p_tx);
            }

            u8_t freed;

            // chop first pbuf from chain
            do
            {
                // try hard to free pbuf 
                freed = pbuf_free(ptr);
            } while (freed == 0);
            // we can read more data now
            tcp_recved(_tpcb, plen);
        }
        else if (wr_err == ERR_MEM)
        {
            // we are low on memory, try later / harder, defer to poll
            _ss->p_tx = ptr; //-V1048
        }
        else
        {
        }
    }
}


err_t ClientTCP::CallbackSent(void *_arg, tcp_pcb *_tpcb, u16_t /*_len*/)
{
    Client *ss = (Client *)_arg;

    if (ss->p_tx != nullptr)
    {
        Send(_tpcb, ss);
    }
    else
    {
        // no more pbufs to send
        if (ss->state == S_CLOSING)
        {
            CloseConnection(_tpcb, ss);
        }
    }

    return ERR_OK;
}


err_t ClientTCP::CallbackRecv(void *_arg, tcp_pcb *_tpcb, pbuf *_p, err_t err)
{
    Client *ss = (Client *)_arg;

    if (_p == nullptr)
    {
        // remote host closed connection
        ss->state = S_CLOSING;
        if (ss->p_tx == nullptr)
        {
            // we're done sending, close it
            CloseConnection(_tpcb, ss);
        }
        else
        {
            // we're not done yet
            tcp_sent(_tpcb, CallbackSent);
        }

        err = ERR_OK;
    }
    else if (err != ERR_OK)
    {
        // cleanup, for unkown reason
        if (_p != nullptr) //-V547
        {
            ss->p_tx = nullptr;
            pbuf_free(_p);
        }
    }
    else if (ss->state == S_RECIEVED || ss->state == S_ACCEPTED)
    {
        ss->state = S_RECIEVED;

        // read some more data
        if (ss->p_tx == nullptr)
        {
            //ss->p = _p;
            tcp_sent(_tpcb, CallbackSent);
            //Send(_tpcb, ss);
            if (SocketFuncReciever)
            {
                SocketFuncReciever((char *)_p->payload, _p->len);
            }

            u8_t freed = 0;
            do
            {
                // try hard to free pbuf 
                freed = pbuf_free(_p);
            } while (freed == 0);

        }
        else
        {
            pbuf *ptr;
            // chain pbufs to the end of what we recv'ed previously
            ptr = ss->p_tx;
            pbuf_chain(ptr, _p);
        }

        err = ERR_OK; //-V1048
    }
    else if (ss->state == S_CLOSING)
    {
        // odd case, remote side closing twice, trash data
        tcp_recved(_tpcb, _p->tot_len);
        ss->p_tx = nullptr;
        pbuf_free(_p);

        err = ERR_OK; //-V1048

        CloseConnection(_tpcb, ss);
    }
    else
    {
        // unknown ss->state, trash data
        tcp_recved(_tpcb, _p->tot_len);
        ss->p_tx = nullptr;
        pbuf_free(_p);

        err = ERR_OK; //-V1048
    }

    return err;
}


void ClientTCP::CallbackError(void *_arg, err_t)
{
    Client *ss = (Client *)_arg;

    pcbClient = nullptr;

    if (ss)
    {
        mem_free(ss);
    }
    
    //tcp_close(_tpcb);
}


err_t ClientTCP::CallbackPoll(void *_arg, tcp_pcb *_tpcb)
{
    err_t ret_err;
    Client *ss = (Client *)_arg;
    if (ss != nullptr)
    {
        if (ss->p_tx != nullptr)
        {
            // there is a remaining pbuf (chain)
            tcp_sent(_tpcb, CallbackSent);
            Send(_tpcb, ss);
        }
        else
        {
            // no remaining pbuf (chain)
            if (ss->state == S_CLOSING)
            {
                CloseConnection(_tpcb, ss);
            }
        }
        ret_err = ERR_OK;
    }
    else
    {
        // nothing to be done
        tcp_abort(_tpcb);
        ret_err = ERR_ABRT;
    }
    return ret_err;
}


err_t ClientTCP::CallbackAccept(void * /*_arg*/, tcp_pcb *_newPCB, err_t err)
{
    /* Unless this pcb should have NORMAL priority, set its priority now.
        When running out of pcbs, low priority pcbs can be aborted to create
        new pcbs of higher priority. */
    tcp_setprio(_newPCB, TCP_PRIO_MIN);

    Client *s = (Client *)mem_malloc(sizeof(Client));

    if (s)
    {
        s->state = S_ACCEPTED;
        s->p_tx = nullptr;
        /* pass newly allocated s to our callbacks */
        tcp_arg(_newPCB, s);
        tcp_recv(_newPCB, CallbackRecv);
        tcp_err(_newPCB, CallbackError);
        tcp_poll(_newPCB, CallbackPoll, 0);
        tcp_sent(_newPCB, CallbackSent);

        err = ERR_OK;

        if (pcbClient == nullptr)
        {
            pcbClient = _newPCB;
            s->state = S_RECIEVED;
        }
    }
    else
    {
        err = ERR_MEM;
    }

    return err;
}


void ClientTCP::SendBuffer(pchar buffer, uint length)
{
    if(pcbClient)
    {
        pbuf *tcpBuffer = pbuf_alloc(PBUF_RAW, (uint16)length, PBUF_POOL);
        tcpBuffer->flags = 1;
        pbuf_take(tcpBuffer, buffer, (uint16)length);
        Client *ss = (Client *)mem_malloc(sizeof(Client));
        ss->p_tx = tcpBuffer;
        Send(pcbClient, ss);
        mem_free(ss);
    }
}


void ClientTCP::SendString(char *format, ...)
{
#undef SIZE_BUFFER
#define SIZE_BUFFER 200

    if(pcbClient)
    {
        static char buffer[SIZE_BUFFER];
        va_list args;
        va_start(args, format);
        vsprintf(buffer, format, args);
        va_end(args);
        std::strcat(buffer, "\r\n");
        SendBuffer(buffer, (uint)std::strlen(buffer));
    }
}


void ClientTCP::SendStringRAW(char *format, ...)
{
#undef SIZE_BUFFER
#define SIZE_BUFFER 200

    if (pcbClient)
    {
        static char buffer[SIZE_BUFFER];
        va_list args;
        va_start(args, format);
        vsprintf(buffer, format, args);
        va_end(args);
        SendBuffer(buffer, (uint)std::strlen(buffer));
    }
}
