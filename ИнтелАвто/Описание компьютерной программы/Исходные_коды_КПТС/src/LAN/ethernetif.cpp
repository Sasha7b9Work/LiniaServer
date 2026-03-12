#include "defines.h"
#include "stm32f4xx_hal.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"
#include "ethernetif.h"
#include "Settings/Settings.h"
#include "LAN/ServerTCP.h"
#include <cstring>

#define DP83848_PHYSCSR_AUTONEGO_DONE   ((uint16_t)0x010U)
#define DP83848_PHYSCSR_HCDSPEEDMASK    ((uint16_t)0x006U)
#define DP83848_PHYSCSR_10BT_HD         ((uint16_t)0x002U)
#define DP83848_PHYSCSR_10BT_FD         ((uint16_t)0x006U)
#define DP83848_PHYSCSR_100BTX_HD       ((uint16_t)0x000U)
#define DP83848_PHYSCSR_100BTX_FD       ((uint16_t)0x004U)

#define DP83848_BCR_SOFT_RESET         ((uint16_t)0x8000U)
#define DP83848_BCR_LOOPBACK           ((uint16_t)0x4000U)
#define DP83848_BCR_SPEED_SELECT       ((uint16_t)0x2000U)
#define DP83848_BCR_AUTONEGO_EN        ((uint16_t)0x1000U)
#define DP83848_BCR_POWER_DOWN         ((uint16_t)0x0800U)
#define DP83848_BCR_ISOLATE            ((uint16_t)0x0400U)
#define DP83848_BCR_RESTART_AUTONEGO   ((uint16_t)0x0200U)
#define DP83848_BCR_DUPLEX_MODE        ((uint16_t)0x0100U)

#define DP83848_BSR_100BASE_T4       ((uint16_t)0x8000U)
#define DP83848_BSR_100BASE_TX_FD    ((uint16_t)0x4000U)
#define DP83848_BSR_100BASE_TX_HD    ((uint16_t)0x2000U)
#define DP83848_BSR_10BASE_T_FD      ((uint16_t)0x1000U)
#define DP83848_BSR_10BASE_T_HD      ((uint16_t)0x0800U)
#define DP83848_BSR_MF_PREAMBLE      ((uint16_t)0x0040U)
#define DP83848_BSR_AUTONEGO_CPLT    ((uint16_t)0x0020U)
#define DP83848_BSR_REMOTE_FAULT     ((uint16_t)0x0010U)
#define DP83848_BSR_AUTONEGO_ABILITY ((uint16_t)0x0008U)
#define DP83848_BSR_LINK_STATUS      ((uint16_t)0x0004U)
#define DP83848_BSR_JABBER_DETECT    ((uint16_t)0x0002U)
#define DP83848_BSR_EXTENDED_CAP     ((uint16_t)0x0001U)

#define DP83848_SMR_MODE       ((uint16_t)0x00E0U)
#define DP83848_SMR_PHY_ADDR   ((uint16_t)0x001FU)

#define DP83848_BCR      ((uint16_t)0x0000U)
#define DP83848_BSR      ((uint16_t)0x0001U)
#define DP83848_PHYI1R   ((uint16_t)0x0002U)
#define DP83848_PHYI2R   ((uint16_t)0x0003U)
#define DP83848_ANAR     ((uint16_t)0x0004U)
#define DP83848_ANLPAR   ((uint16_t)0x0005U)
#define DP83848_ANER     ((uint16_t)0x0006U)
#define DP83848_ANNPTR   ((uint16_t)0x0007U)
#define DP83848_SMR      ((uint16_t)0x0019U)
#define DP83848_ISFR     ((uint16_t)0x0012U)
#define DP83848_IMR      ((uint16_t)0x0011U)
#define DP83848_PHYSCSR  ((uint16_t)0x0010U)

#define DP83848_MAX_DEV_ADDR   ((uint32_t)31U)

#define  DP83848_STATUS_READ_ERROR            ((int32_t)-5)
#define  DP83848_STATUS_WRITE_ERROR           ((int32_t)-4)
#define  DP83848_STATUS_ADDRESS_ERROR         ((int32_t)-3)
#define  DP83848_STATUS_RESET_TIMEOUT         ((int32_t)-2)
#define  DP83848_STATUS_ERROR                 ((int32_t)-1)
#define  DP83848_STATUS_OK                    ((int32_t) 0)
#define  DP83848_STATUS_LINK_DOWN             ((int32_t) 1)
#define  DP83848_STATUS_100MBITS_FULLDUPLEX   ((int32_t) 2)
#define  DP83848_STATUS_100MBITS_HALFDUPLEX   ((int32_t) 3)
#define  DP83848_STATUS_10MBITS_FULLDUPLEX    ((int32_t) 4)
#define  DP83848_STATUS_10MBITS_HALFDUPLEX    ((int32_t) 5)
#define  DP83848_STATUS_AUTONEGO_NOTDONE      ((int32_t) 6)

#define IFNAME0 's'
#define IFNAME1 't'

#define ETH_DMA_TRANSMIT_TIMEOUT                (20U)

#define ETH_RX_BUFFER_CNT           10     /* This app buffers receive packets of its primary service
                                               * protocol for processing later. */
#define ETH_TX_BUFFER_MAX             ((ETH_TX_DESC_CNT) * 2) /* HAL_ETH_Transmit(_IT) may attach two
                                               * buffers per descriptor. */

typedef int32_t  (*dp83848_Init_Func) (void);
typedef int32_t  (*dp83848_DeInit_Func) (void);
typedef int32_t  (*dp83848_ReadReg_Func)   (uint32_t, uint32_t, uint32_t *);
typedef int32_t  (*dp83848_WriteReg_Func)  (uint32_t, uint32_t, uint32_t);
typedef int32_t  (*dp83848_GetTick_Func)  (void);

typedef struct
{
  dp83848_Init_Func      Init;
  dp83848_DeInit_Func    DeInit;
  dp83848_WriteReg_Func  WriteReg;
  dp83848_ReadReg_Func   ReadReg;
  dp83848_GetTick_Func   GetTick;
} dp83848_IOCtx_t;

typedef struct
{
  uint32_t            DevAddr;
  uint32_t            Is_Initialized;
  dp83848_IOCtx_t     IO;
  void               *pData;
}dp83848_Object_t;

/**
  * @brief  Register IO functions to component object
  * @param  pObj: device object  of DP83848_Object_t.
  * @param  ioctx: holds device IO functions.
  * @retval DP83848_STATUS_OK  if OK
  *         DP83848_STATUS_ERROR if missing mandatory function
  */
int32_t  DP83848_RegisterBusIO(dp83848_Object_t *pObj, dp83848_IOCtx_t *ioctx)
{
  if(!pObj || !ioctx->ReadReg || !ioctx->WriteReg || !ioctx->GetTick)
  {
    return DP83848_STATUS_ERROR;
  }

  pObj->IO.Init = ioctx->Init;
  pObj->IO.DeInit = ioctx->DeInit;
  pObj->IO.ReadReg = ioctx->ReadReg;
  pObj->IO.WriteReg = ioctx->WriteReg;
  pObj->IO.GetTick = ioctx->GetTick;

  return DP83848_STATUS_OK;
}

/**
  * @brief  Initialize the DP83848 and configure the needed hardware resources
  * @param  pObj: device object DP83848_Object_t.
  * @retval DP83848_STATUS_OK  if OK
  *         DP83848_STATUS_ADDRESS_ERROR if cannot find device address
  *         DP83848_STATUS_READ_ERROR if connot read register
  */
 int32_t DP83848_Init(dp83848_Object_t *pObj)
 {
   uint32_t regvalue = 0, addr = 0;
   int32_t status = DP83848_STATUS_OK;

   if(pObj->Is_Initialized == 0)
   {
     if(pObj->IO.Init != nullptr)
     {
       /* GPIO and Clocks initialization */
       pObj->IO.Init();
     }

     /* for later check */
     pObj->DevAddr = DP83848_MAX_DEV_ADDR + 1;

     /* Get the device address from special mode register */
     for(addr = 0; addr <= DP83848_MAX_DEV_ADDR; addr ++)
     {
       if(pObj->IO.ReadReg(addr, DP83848_SMR, &regvalue) < 0)
       {
         status = DP83848_STATUS_READ_ERROR;
         /* Can't read from this device address
            continue with next address */
         continue;
       }

       if((regvalue & DP83848_SMR_PHY_ADDR) == addr)
       {
         pObj->DevAddr = addr;
         status = DP83848_STATUS_OK;
         break;
       }
     }

     if(pObj->DevAddr > DP83848_MAX_DEV_ADDR)
     {
       status = DP83848_STATUS_ADDRESS_ERROR;
     }

     /* if device address is matched */
     if(status == DP83848_STATUS_OK)
     {
       pObj->Is_Initialized = 1;
     }
   }

   return status;
 }

/**
  * @brief  Get the link state of DP83848 device.
  * @param  pObj: Pointer to device object.
  * @param  pLinkState: Pointer to link state
  * @retval DP83848_STATUS_LINK_DOWN  if link is down
  *         DP83848_STATUS_AUTONEGO_NOTDONE if Auto nego not completed
  *         DP83848_STATUS_100MBITS_FULLDUPLEX if 100Mb/s FD
  *         DP83848_STATUS_100MBITS_HALFDUPLEX if 100Mb/s HD
  *         DP83848_STATUS_10MBITS_FULLDUPLEX  if 10Mb/s FD
  *         DP83848_STATUS_10MBITS_HALFDUPLEX  if 10Mb/s HD
  *         DP83848_STATUS_READ_ERROR if connot read register
  *         DP83848_STATUS_WRITE_ERROR if connot write to register
  */
int32_t DP83848_GetLinkState(dp83848_Object_t *pObj)
{
  uint32_t readval = 0;

  /* Read Status register  */
  if(pObj->IO.ReadReg(pObj->DevAddr, DP83848_BSR, &readval) < 0)
  {
    return DP83848_STATUS_READ_ERROR;
  }

  /* Read Status register again */
  if(pObj->IO.ReadReg(pObj->DevAddr, DP83848_BSR, &readval) < 0)
  {
    return DP83848_STATUS_READ_ERROR;
  }

  if((readval & DP83848_BSR_LINK_STATUS) == 0)
  {
    /* Return Link Down status */
    return DP83848_STATUS_LINK_DOWN;
  }

  /* Check Auto negotiaition */
  if(pObj->IO.ReadReg(pObj->DevAddr, DP83848_BCR, &readval) < 0)
  {
    return DP83848_STATUS_READ_ERROR;
  }

  if((readval & DP83848_BCR_AUTONEGO_EN) != DP83848_BCR_AUTONEGO_EN)
  {
    if(((readval & DP83848_BCR_SPEED_SELECT) == DP83848_BCR_SPEED_SELECT) && ((readval & DP83848_BCR_DUPLEX_MODE) == DP83848_BCR_DUPLEX_MODE))
    {
      return DP83848_STATUS_100MBITS_FULLDUPLEX;
    }
    else if ((readval & DP83848_BCR_SPEED_SELECT) == DP83848_BCR_SPEED_SELECT)
    {
      return DP83848_STATUS_100MBITS_HALFDUPLEX;
    }
    else if ((readval & DP83848_BCR_DUPLEX_MODE) == DP83848_BCR_DUPLEX_MODE)
    {
      return DP83848_STATUS_10MBITS_FULLDUPLEX;
    }
    else
    {
      return DP83848_STATUS_10MBITS_HALFDUPLEX;
    }
  }
  else /* Auto Nego enabled */
  {
    if(pObj->IO.ReadReg(pObj->DevAddr, DP83848_PHYSCSR, &readval) < 0)
    {
      return DP83848_STATUS_READ_ERROR;
    }

    /* Check if auto nego not done */
    if((readval & DP83848_PHYSCSR_AUTONEGO_DONE) == 0)
    {
      return DP83848_STATUS_AUTONEGO_NOTDONE;
    }

    if((readval & DP83848_PHYSCSR_HCDSPEEDMASK) == DP83848_PHYSCSR_100BTX_FD)
    {
      return DP83848_STATUS_100MBITS_FULLDUPLEX;
    }
    else if ((readval & DP83848_PHYSCSR_HCDSPEEDMASK) == DP83848_PHYSCSR_100BTX_HD)
    {
      return DP83848_STATUS_100MBITS_HALFDUPLEX;
    }
    else if ((readval & DP83848_PHYSCSR_HCDSPEEDMASK) == DP83848_PHYSCSR_10BT_FD)
    {
      return DP83848_STATUS_10MBITS_FULLDUPLEX;
    }
    else
    {
      return DP83848_STATUS_10MBITS_HALFDUPLEX;
    }
  }
}

/**
@Note: This interface is implemented to operate in zero-copy mode only:
        - Rx Buffers will be allocated from LwIP stack Rx memory pool,
          then passed to ETH HAL driver.
        - Tx Buffers will be allocated from LwIP stack memory heap,
          then passed to ETH HAL driver.

@Notes:
  1.a. ETH DMA Rx descriptors must be contiguous, the default count is 4,
       to customize it please redefine ETH_RX_DESC_CNT in ETH GUI (Rx Descriptor Length)
       so that updated value will be generated in stm32xxxx_hal_conf.h
  1.b. ETH DMA Tx descriptors must be contiguous, the default count is 4,
       to customize it please redefine ETH_TX_DESC_CNT in ETH GUI (Tx Descriptor Length)
       so that updated value will be generated in stm32xxxx_hal_conf.h

  2.a. Rx Buffers number: ETH_RX_BUFFER_CNT must be greater than ETH_RX_DESC_CNT.
  2.b. Rx Buffers must have the same size: ETH_RX_BUF_SIZE, this value must
       passed to ETH DMA in the init field (heth.Init.RxBuffLen)
*/
typedef enum
{
  RX_ALLOC_OK       = 0x00,
  RX_ALLOC_ERROR    = 0x01
} RxAllocStatusTypeDef;

#ifdef WIN32
    #define __ALIGNED(x)
#endif

struct RxBuff_t
{
  struct pbuf_custom pbuf_custom;
  uint8_t buff[(ETH_RX_BUF_SIZE + 31) & ~31] __ALIGNED(32);
};

ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */

ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

/* Memory Pool Declaration */
LWIP_MEMPOOL_DECLARE(RX_POOL, ETH_RX_BUFFER_CNT, sizeof(RxBuff_t), "Zero-copy RX PBUF pool");

static uint8_t RxAllocStatus;

/* Global Ethernet handle*/
ETH_HandleTypeDef EthHandle;
ETH_TxPacketConfig TxConfig;
dp83848_Object_t DP83848;

/* Private function prototypes -----------------------------------------------*/
extern void Error_Handler(void);
int32_t ETH_PHY_IO_Init(void);
int32_t ETH_PHY_IO_DeInit (void);
int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal);
int32_t ETH_PHY_IO_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal);
int32_t ETH_PHY_IO_GetTick(void);
void pbuf_free_custom(struct pbuf *p);

dp83848_IOCtx_t  DP83848_IOCtx = {ETH_PHY_IO_Init,
                               ETH_PHY_IO_DeInit,
                               ETH_PHY_IO_WriteReg,
                               ETH_PHY_IO_ReadReg,
                               ETH_PHY_IO_GetTick};
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
                       LL Driver Interface ( LwIP stack --> ETH) 
*******************************************************************************/
/**
  * @brief In this function, the hardware should be initialized.
  * Called from ethernetif_init().
  *
  * @param netif the already initialized lwip network interface structure
  *        for this ethernetif
  */
static void low_level_init(struct netif *netif)
{
  static uint8_t macaddress[6]= {gset.mac[0], gset.mac[1], gset.mac[2], gset.mac[3], gset.mac[4], gset.mac[5] };

  for (int i = 0; i < 6; i++)
  {
      macaddress[i] = gset.mac[i];
    }

  EthHandle.Instance = ETH;
  EthHandle.Init.MACAddr = macaddress;
  EthHandle.Init.MediaInterface = HAL_ETH_MII_MODE;
  EthHandle.Init.RxDesc = DMARxDscrTab;
  EthHandle.Init.TxDesc = DMATxDscrTab;
  EthHandle.Init.RxBuffLen = ETH_RX_BUF_SIZE;

  /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
  HAL_ETH_Init(&EthHandle);

  /* set MAC hardware address length */
  netif->hwaddr_len = ETH_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] =  gset.mac[0];
  netif->hwaddr[1] =  gset.mac[1];
  netif->hwaddr[2] =  gset.mac[2];
  netif->hwaddr[3] =  gset.mac[3];
  netif->hwaddr[4] =  gset.mac[4];
  netif->hwaddr[5] =  gset.mac[5];

  /* maximum transfer unit */
  netif->mtu = ETH_MAX_PAYLOAD;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

  /* Initialize the RX POOL */
  LWIP_MEMPOOL_INIT(RX_POOL);

  /* Set Tx packet config common parameters */
  std::memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;

  /* Set PHY IO functions */
  DP83848_RegisterBusIO(&DP83848, &DP83848_IOCtx);

  /* Initialize the DP83848 ETH PHY */
  if(DP83848_Init(&DP83848) != DP83848_STATUS_OK)
  {
    netif_set_link_down(netif);
    netif_set_down(netif);
    return;
  }

  ethernet_link_check_state(netif);
}

/**
  * @brief This function should do the actual transmission of the packet. The packet is
  * contained in the pbuf that is passed to the function. This pbuf
  * might be chained.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
  * @return ERR_OK if the packet could be sent
  *         an err_t value if the packet couldn't be sent
  *
  * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
  *       strange results. You might consider waiting for space in the DMA queue
  *       to become available since the stack doesn't retry to send a packet
  *       dropped because of memory failure (except for the TCP timers).
  */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  (void)netif;
  uint32_t i = 0U;
  struct pbuf *q = nullptr;
  err_t errval = ERR_OK;
  static ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT];

  std::memset(Txbuffer, 0 , ETH_TX_DESC_CNT*sizeof(ETH_BufferTypeDef));

  for(q = p; q != nullptr; q = q->next)
  {
    if(i >= ETH_TX_DESC_CNT)
      return ERR_IF;

    Txbuffer[i].buffer = (uint8 *)q->payload;
    Txbuffer[i].len = q->len;

    if(i>0)
    {
      Txbuffer[i-1].next = &Txbuffer[i];
    }

    if(q->next == nullptr)
    {
      Txbuffer[i].next = nullptr;
    }

    i++;
  }

  TxConfig.Length = p->tot_len;
  TxConfig.TxBuffer = Txbuffer;
  TxConfig.pData = p;

  HAL_ETH_Transmit(&EthHandle, &TxConfig, ETH_DMA_TRANSMIT_TIMEOUT);

  return errval;
}

/**
  * @brief Should allocate a pbuf and transfer the bytes of the incoming
  * packet from the interface into the pbuf.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @return a pbuf filled with the received packet (including MAC header)
  *         NULL on memory error
  */
static struct pbuf * low_level_input(struct netif *netif)
{
  (void)netif;
  struct pbuf *p = nullptr;

  if(RxAllocStatus == RX_ALLOC_OK)
  {
    HAL_ETH_ReadData(&EthHandle, (void **)&p);
  }
  return p;
}
/**
  * @brief This function should be called when a packet is ready to be read
  * from the interface. It uses the function low_level_input() that
  * should handle the actual reception of bytes from the network
  * interface. Then the type of the received packet is determined and
  * the appropriate input function is called.
  *
  * @param netif the lwip network interface structure for this ethernetif
  */
void ethernetif_input(struct netif *netif)
{
  struct pbuf *p = nullptr;

    do
    {
      p = low_level_input( netif );
      if (p != nullptr)
      {
        if (netif->input( p, netif) != ERR_OK )
        {
          pbuf_free(p);
        }
      }

    } while(p!=nullptr);

}

/**
  * @brief Should be called at the beginning of the program to set up the
  * network interface. It calls the function low_level_init() to do the
  * actual setup of the hardware.
  *
  * This function should be passed as a parameter to netif_add().
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @return ERR_OK if the loopif is initialized
  *         ERR_MEM if private data couldn't be allocated
  *         any other err_t on error
  */
err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

/**
  * @brief  Custom Rx pbuf free callback
  * @param  pbuf: pbuf to be freed
  * @retval None
  */
void pbuf_free_custom(struct pbuf *p)
{
  struct pbuf_custom* custom_pbuf = (struct pbuf_custom*)p;
  LWIP_MEMPOOL_FREE(RX_POOL, custom_pbuf);
   /* If the Rx Buffer Pool was exhausted, signal the ethernetif_input task to
   * call HAL_ETH_GetRxDataBuffer to rebuild the Rx descriptors. */
  if (RxAllocStatus == RX_ALLOC_ERROR)
  {
    RxAllocStatus = RX_ALLOC_OK;
  }
}

/**
  * @brief  Returns the current time in milliseconds
  *         when LWIP_TIMERS == 1 and NO_SYS == 1
  * @param  None
  * @retval Current Time value
  */

#ifdef __cplusplus
extern "C" {
#endif

u32_t sys_now(void)
{
  return HAL_GetTick();
}

#ifdef __cplusplus
}
#endif

/*******************************************************************************
                       Ethernet MSP Routines
*******************************************************************************/
/**
  * @brief  Initializes the ETH MSP.
  * @param  heth: ETH handle
  * @retval None
  */
void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
  (void)heth;
  GPIO_InitTypeDef GPIO_InitStructure = {0};

  /* Enable GPIOs clocks */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE(); 

/* Ethernet pins configuration ************************************************/
  /*
        ETH_MDIO -------------------------> PA2 +
        ETH_MDC --------------------------> PC1 +
        ETH_PPS_OUT ----------------------> PB5 -  -
        ETH_MII_RXD2 ---------------------> PH6 - PB0
        ETH_MII_RXD3 ---------------------> PH7 - PB1
        ETH_MII_TX_CLK -------------------> PC3 +
        ETH_MII_TXD2 ---------------------> PC2 +
        ETH_MII_TXD3 ---------------------> PB8 +
        ETH_MII_RX_CLK -------------------> PA1 +
        ETH_MII_RX_DV --------------------> PA7 +
        ETH_MII_RXD0 ---------------------> PC4 +
        ETH_MII_RXD1 ---------------------> PC5 +
        ETH_MII_TX_EN --------------------> PG11 - PB11
        ETH_MII_TXD0 ---------------------> PG13 - PB12
        ETH_MII_TXD1 ---------------------> PG14 - PB13
        ETH_MII_RX_ER --------------------> PI10 (not configured)        
        ETH_MII_CRS ----------------------> PA0  (not configured)
        ETH_MII_COL ----------------------> PH3  (not configured)
  */

  /* Configure PA1, PA2 and PA7 */
  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
  GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure PB5 and PB8 */
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Configure PC1, PC2, PC3, PC4 and PC5 */
  GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  /* Configure PA0 
  GPIO_InitStructure.Pin =  GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  Note: Ethernet Full duplex mode works properly in the default setting
  (which MII_CRS is not connected to PA0 of STM32F4x9NIH6) because PA0 is shared
  with Wakeup button and MC_ENA.
  If Half duplex mode is needed, uncomment PA0 configuration code source (above 
  the note) and close the SB7 solder bridge of the STM324x9I-EVAL board .
  */

  /* Configure PH3 
  GPIO_InitStructure.Pin =  GPIO_PIN_3;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);
  
  Note: Ethernet Full duplex mode works properly in the default setting
  (which MII_COL is not connected to PH3 of STM32F4x9NIH6) because PH3 is shared
  with SDRAM chip select SDNE0. 
  If Half duplex mode is needed, uncomment PH3 configuration code source (above 
  the note) and close SB8 solder bridge of the STM324x9I-EVAL board.
  */

  /* Configure PI10
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStructure); 
  
  Note: Ethernet works properly in the default setting (which RX_ER is not 
  connected to PI10 of STM32F4x9NIH6) because PI10 is shared with data signal
  of SDRAM. 
  If RX_ER signal is needed, uncomment PI10 configuration code source (above 
  the note) then remove R244 and solder R43 of the STM324x9I-EVAL board.
  */
  
  /* Enable ETHERNET clock  */
  __HAL_RCC_ETH_CLK_ENABLE();
}

/*******************************************************************************
                       PHI IO Functions
*******************************************************************************/
/**
  * @brief  Initializes the MDIO interface GPIO and clocks.
  * @param  None
  * @retval 0 if OK, -1 if ERROR
  */
int32_t ETH_PHY_IO_Init(void)
{
  /* We assume that MDIO GPIO configuration is already done
     in the ETH_MspInit() else it should be done here
  */

  /* Configure the MDIO Clock */
  HAL_ETH_SetMDIOClockRange(&EthHandle);

  return 0;
}

/**
  * @brief  De-Initializes the MDIO interface .
  * @param  None
  * @retval 0 if OK, -1 if ERROR
  */
int32_t ETH_PHY_IO_DeInit (void)
{
  return 0;
}

/**
  * @brief  Read a PHY register through the MDIO interface.
  * @param  DevAddr: PHY port address
  * @param  RegAddr: PHY register address
  * @param  pRegVal: pointer to hold the register value
  * @retval 0 if OK -1 if Error
  */
int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal)
{
  if(HAL_ETH_ReadPHYRegister(&EthHandle, DevAddr, RegAddr, pRegVal) != HAL_OK)
  {
    return -1;
  }

  return 0;
}

/**
  * @brief  Write a value to a PHY register through the MDIO interface.
  * @param  DevAddr: PHY port address
  * @param  RegAddr: PHY register address
  * @param  RegVal: Value to be written
  * @retval 0 if OK -1 if Error
  */
int32_t ETH_PHY_IO_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal)
{
  if(HAL_ETH_WritePHYRegister(&EthHandle, DevAddr, RegAddr, RegVal) != HAL_OK)
  {
    return -1;
  }

  return 0;
}

/**
  * @brief  Get the time in millisecons used for internal PHY driver process.
  * @retval Time value
  */
int32_t ETH_PHY_IO_GetTick(void)
{
  return (int)HAL_GetTick();
}

/**
  * @brief
  * @retval None
  */
void ethernet_link_check_state(struct netif *netif)
{
  ETH_MACConfigTypeDef MACConf = {0};
  int32_t PHYLinkState = 0U;
  uint32_t linkchanged = 0U, speed = 0U, duplex =0U;

  PHYLinkState = DP83848_GetLinkState(&DP83848);

  if(netif_is_link_up(netif) && (PHYLinkState <= DP83848_STATUS_LINK_DOWN))
  {
    HAL_ETH_Stop(&EthHandle);
    netif_set_down(netif);
    netif_set_link_down(netif);
  }
  else if(!netif_is_link_up(netif) && (PHYLinkState > DP83848_STATUS_LINK_DOWN))
  {
    switch (PHYLinkState)
    {
    case DP83848_STATUS_100MBITS_FULLDUPLEX:
      duplex = ETH_FULLDUPLEX_MODE;
      speed = ETH_SPEED_100M;
      linkchanged = 1;
      break;
    case DP83848_STATUS_100MBITS_HALFDUPLEX:
      duplex = ETH_HALFDUPLEX_MODE;
      speed = ETH_SPEED_100M;
      linkchanged = 1;
      break;
    case DP83848_STATUS_10MBITS_FULLDUPLEX:
      duplex = ETH_FULLDUPLEX_MODE;
      speed = ETH_SPEED_10M;
      linkchanged = 1;
      break;
    case DP83848_STATUS_10MBITS_HALFDUPLEX:
      duplex = ETH_HALFDUPLEX_MODE;
      speed = ETH_SPEED_10M;
      linkchanged = 1;
      break;
    default:
      break;
    }

    if(linkchanged)
    {
      /* Get MAC Config MAC */
      HAL_ETH_GetMACConfig(&EthHandle, &MACConf);
      MACConf.DuplexMode = duplex;
      MACConf.Speed = speed;
      HAL_ETH_SetMACConfig(&EthHandle, &MACConf);
      HAL_ETH_Start(&EthHandle);
      netif_set_up(netif);
      netif_set_link_up(netif);
    }
  }
}


#ifdef WIN32
    #define offsetof(x, y) 1
#endif


void HAL_ETH_RxAllocateCallback(uint8_t **buff)
{
  struct pbuf_custom *p = (pbuf_custom *)LWIP_MEMPOOL_ALLOC(RX_POOL);
  if (p)
  {
    /* Get the buff from the struct pbuf address. */
    *buff = (uint8_t *)p + offsetof(RxBuff_t, buff);
    p->custom_free_function = pbuf_free_custom;
    /* Initialize the struct pbuf.
    * This must be performed whenever a buffer's allocated because it may be
    * changed by lwIP or the app, e.g., pbuf_free decrements ref. */
    pbuf_alloced_custom(PBUF_RAW, 0, PBUF_REF, p, *buff, ETH_RX_BUF_SIZE);
  }
  else
  {
    RxAllocStatus = RX_ALLOC_ERROR;
    *buff = nullptr;
  }
}

void HAL_ETH_RxLinkCallback(void **pStart, void **pEnd, uint8_t *buff, uint16_t Length)
{
  struct pbuf **ppStart = (struct pbuf **)pStart;
  struct pbuf **ppEnd = (struct pbuf **)pEnd;
  struct pbuf *p = nullptr;

  /* Get the struct pbuf from the buff address. */
  p = (struct pbuf *)(buff - offsetof(RxBuff_t, buff));
  p->next = nullptr;
  p->tot_len = 0;
  p->len = Length;

  /* Chain the buffer. */
  if (!*ppStart)
  {
    /* The first buffer of the packet. */
    *ppStart = p;
  }
  else
  {
    /* Chain the buffer to the end of the packet. */
    (*ppEnd)->next = p;
  }
  *ppEnd  = p;

  /* Update the total length of all the buffers of the chain. Each pbuf in the chain should have its tot_len
   * set to its own length, plus the length of all the following pbufs in the chain. */
  for (p = *ppStart; p != nullptr; p = p->next)
  {
    p->tot_len += Length;
  }

  ServerTCP::CallbackOnRxETH();
}

void HAL_ETH_TxFreeCallback(uint32_t * buff)
{
  pbuf_free((struct pbuf *)buff);
}
