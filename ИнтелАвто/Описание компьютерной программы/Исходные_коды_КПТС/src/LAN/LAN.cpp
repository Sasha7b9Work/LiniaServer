// 2025/02/17 15:25:26 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "LAN/LAN.h"
#include "LAN/ClientTCP.h"
#include "LAN/ServerTCP.h"
#include "LAN/http_cgi_ssi.h"
#include "LAN/ethernetif.h"
#include "LAN/app_ethernet.h"
#include "Settings/Settings.h"
#include "SCPI/SCPI.h"
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/timeouts.h>
#include <netif/ethernet.h>


namespace LAN
{
    static void Netif_Config(void);

    struct netif gnetif;

    // Функция приёма SCPI
    static void FuncReceiverClient(pchar buffer, uint length)
    {
        SCPI::AppendNewData(DirectionSCPI::LAN, (uint8 *)buffer, length);
    }

    static void FuncReceiverServer(pchar /*buffer*/, uint /*length*/)
    {
        ServerTCP::CallbackOnRxETH();
    }
}


void LAN::Init()
{
    /* Initialize the LwIP stack */
    lwip_init();

    /* Configure the Network interface */
    Netif_Config();

    /* Http webserver Init */
    http_server_init();

    Update();

    ClientTCP::Init(FuncReceiverClient);
}


void LAN::Update()
{
    /* Read a received packet from the Ethernet buffers and send it
       to the lwIP for handling */
    ethernetif_input(&gnetif);

    /* Handle timeouts */
    sys_check_timeouts();

#if LWIP_NETIF_LINK_CALLBACK
    Ethernet_Link_Periodic_Handle(&gnetif);
#endif

    if (!ServerTCP::IsConnected())
    {
        ServerTCP::Init(FuncReceiverServer);
    }
}


void LAN::Netif_Config(void)
{
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

    /* IP address default setting */
    IP4_ADDR(&ipaddr, gset.ipSCPI[0], gset.ipSCPI[1], gset.ipSCPI[2], gset.ipSCPI[3]);
    IP4_ADDR(&netmask, gset.netmask[0], gset.netmask[1], gset.netmask[2], gset.netmask[3]);
    IP4_ADDR(&gw, gset.gw[0], gset.gw[1], gset.gw[2], gset.gw[3]);

    /* add the network interface */
    netif_add(&gnetif, &ipaddr, &netmask, &gw, nullptr, &ethernetif_init, &ethernet_input);

    /*  Registers the default network interface */
    netif_set_default(&gnetif);

    ethernet_link_status_updated(&gnetif);

#if LWIP_NETIF_LINK_CALLBACK
    netif_set_link_callback(&gnetif, ethernet_link_status_updated);
#endif
}
