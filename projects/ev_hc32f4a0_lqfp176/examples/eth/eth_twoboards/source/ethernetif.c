/**
 *******************************************************************************
 * @file  eth/eth_twoboards/source/ethernetif.c
 * @brief This file implements Ethernet network interface drivers.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2023-09-30       CDT             Add MAC interface selection before ETH_DeInit function
   2024-11-08       CDT             Optimize the init process of ETH
                                    Add high driver for ETH output pin
 @endverbatim
 *******************************************************************************
 * Copyright (C) 2022-2025, Xiaohua Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by XHSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "main.h"

/**
 * @addtogroup HC32F4A0_DDL_Examples
 * @{
 */

/**
 * @addtogroup ETH_Two_Boards
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Set pin to low at least 10ms */
#define PHY_HW_RST_DELAY                (10U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
/* Global Ethernet handle*/
static stc_eth_handle_t EthHandle;
/* Ethernet Tx DMA Descriptor */
__ALIGN_BEGIN static stc_eth_dma_desc_t EthDmaTxDscrTab[ETH_TX_BUF_NUM];
/* Ethernet Rx DMA Descriptor */
__ALIGN_BEGIN static stc_eth_dma_desc_t EthDmaRxDscrTab[ETH_RX_BUF_NUM];
/* Ethernet Transmit Buffer */
__ALIGN_BEGIN static uint8_t EthTxBuff[ETH_TX_BUF_NUM][ETH_TX_BUF_SIZE];
/* Ethernet Receive Buffer */
__ALIGN_BEGIN static uint8_t EthRxBuff[ETH_RX_BUF_NUM][ETH_RX_BUF_SIZE];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @defgroup ETH_IF_Global_Functions Ethernet Interface Global Functions
 * @{
 */

/**
 * @brief  Initializes the Ethernet GPIO.
 * @param  None
 * @retval None
 */
static void Ethernet_GpioInit(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    /* Holding ETH_RST */
    BSP_IO_ConfigPortPin(EIO_PORT1, EIO_ETH_RST, EIO_DIR_OUT);
    BSP_IO_WritePortPin(EIO_PORT1, EIO_ETH_RST, EIO_PIN_RESET);
    SysTick_Delay(PHY_HW_RST_DELAY);

    /* Configure MII selection IO for ETH */
    /**
        ETH_MII_TX_CLK --------------> PB6
        ETH_MII_TX_EN ---------------> PG11
        ETH_MII_TXD0 ----------------> PG13
        ETH_MII_TXD1 ----------------> PG14
        ETH_MII_TXD2 ----------------> PB9
        ETH_MII_TXD3 ----------------> PB8
        ETH_MII_RX_CLK --------------> PA1
        ETH_MII_RX_DV ---------------> PA7
        ETH_MII_RXD0 ----------------> PC4
        ETH_MII_RXD1 ----------------> PC5
        ETH_MII_RXD2 ----------------> PB0
        ETH_MII_RXD3 ----------------> PB1
    */
    GPIO_Init(GPIO_PORT_B, (GPIO_PIN_08 | GPIO_PIN_09), &stcGpioInit);
    GPIO_Init(GPIO_PORT_G, (GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14), &stcGpioInit);
    /* Configure PA1 and PA7 */
    GPIO_SetFunc(GPIO_PORT_A, (GPIO_PIN_01 | GPIO_PIN_07), GPIO_FUNC_11);
    /* Configure PB0, PB1, PB6, PB8 and PB9 */
    GPIO_SetFunc(GPIO_PORT_B, (GPIO_PIN_00 | GPIO_PIN_01 | GPIO_PIN_06 | GPIO_PIN_08 | GPIO_PIN_09), GPIO_FUNC_11);
    /* Configure PC4 and PC5 */
    GPIO_SetFunc(GPIO_PORT_C, (GPIO_PIN_04 | GPIO_PIN_05), GPIO_FUNC_11);
    /* Configure PG11, PG13 and PG14 */
    GPIO_SetFunc(GPIO_PORT_G, (GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14), GPIO_FUNC_11);
}

/**
 * @brief  In this function, the hardware should be initialized.
 * @param  None
 * @retval int32_t:
 *           - LL_OK: Initialize success
 *           - LL_ERR: Initialize failed
 */
static int32_t low_level_init(void)
{
    stc_eth_init_t stcEthInit;

    /* Enable ETH clock */
    FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_ETHMAC, ENABLE);
    /* Init Ethernet GPIO */
    Ethernet_GpioInit();
    /* Configure structure initialization */
    (void)ETH_CommStructInit(&EthHandle.stcCommInit);
    (void)ETH_StructInit(&stcEthInit);
    EthHandle.stcCommInit.u32Interface      = ETH_MAC_IF_MII;
    EthHandle.stcCommInit.u32ChecksumMode   = ETH_MAC_CHECKSUM_MD_SW;
    /* DMA */
    stcEthInit.stcDmaInit.u32RxBurstLen          = ETH_DMA_RX_BURST_LEN_1BEAT;
    stcEthInit.stcDmaInit.u32TxBurstLen          = ETH_DMA_TX_BURST_LEN_1BEAT;
    stcEthInit.stcDmaInit.u32DropChecksumErrorFrame    = ETH_DMA_DROP_CHECKSUM_ERR_FRAME_DISABLE;
    stcEthInit.stcDmaInit.u32ForwardErrorFrame         = ETH_DMA_FORWARD_ERR_FRAME_ENABLE;
    stcEthInit.stcDmaInit.u32ForwardUndersizeFrame = ETH_DMA_FORWARD_UNDERSIZE_FRAME_ENABLE;
    /* MAC */
    stcEthInit.stcMacInit.u32AutoStripPadFCS      = ETH_MAC_AUTO_STRIP_PAD_FCS_ENABLE;
    stcEthInit.stcMacInit.u32ReceiveOwn           = ETH_MAC_RX_OWN_DISABLE;
    stcEthInit.stcMacInit.u32ReceiveAll           = ETH_MAC_RX_ALL_ENABLE;
    stcEthInit.stcMacInit.u32PassControlFrame     = ETH_MAC_PASS_CTRL_FRAME_FORWARD_ALL;

    /* Select MII Mode*/
    MODIFY_REG32(CM_ETH->MAC_IFCONFR, ETH_MAC_IFCONFR_IFSEL, EthHandle.stcCommInit.u32Interface);
    /* Set communication mode */
    ETH_MAC_SetDuplexSpeed(EthHandle.stcCommInit.u32DuplexMode, EthHandle.stcCommInit.u32Speed);
    /* Configure MAC and DMA */
    (void)ETH_MAC_Init(&EthHandle, &stcEthInit.stcMacInit);
    (void)ETH_DMA_Init(&stcEthInit.stcDmaInit);
    /* Initialize Tx Descriptors list: Chain Mode */
    (void)ETH_DMA_TxDescListInit(&EthHandle, EthDmaTxDscrTab, &EthTxBuff[0][0], ETH_TX_BUF_NUM);
    /* Initialize Rx Descriptors list: Chain Mode  */
    (void)ETH_DMA_RxDescListInit(&EthHandle, EthDmaRxDscrTab, &EthRxBuff[0][0], ETH_RX_BUF_NUM);
    /* Enable MAC and DMA transmission and reception */
    (void)ETH_Start();

    return LL_OK;
}

/**
 * @brief  This function should do the actual transmission of the packet.
 * @param  p                            The data packet to send
 * @retval int32_t:
 *           - LL_OK: The packet could be sent
 *           - LL_ERR: The packet couldn't be sent
 */
int32_t low_level_output(struct pbuf *p)
{
    int32_t i32Ret;
    struct pbuf *q;
    uint8_t *txBuffer;
    __IO stc_eth_dma_desc_t *DmaTxDesc;
    uint32_t byteCnt;
    uint32_t frameLength = 0UL;
    uint32_t bufferOffset;
    uint32_t payloadOffset;

    DmaTxDesc = EthHandle.stcTxDesc;
    txBuffer = (uint8_t *)((EthHandle.stcTxDesc)->u32Buf1Addr);
    bufferOffset = 0UL;
    /* Copy frame from pbufs to driver buffers */
    for (q = p; q != NULL; q = q->next) {
        /* If this buffer isn't available, goto error */
        if (0UL != (DmaTxDesc->u32ControlStatus & ETH_DMA_TXDESC_OWN)) {
            i32Ret = LL_ERR;
            goto error;
        }

        /* Get bytes in current buffer */
        byteCnt = q->len;
        payloadOffset = 0UL;
        /* Check if the length of data to copy is bigger than Tx buffer size */
        while ((byteCnt + bufferOffset) > ETH_TX_BUF_SIZE) {
            /* Copy data to Tx buffer*/
            (void)memcpy((uint8_t *) & (txBuffer[bufferOffset]), (uint8_t *) & (((uint8_t *)q->payload)[payloadOffset]), (ETH_TX_BUF_SIZE - bufferOffset));
            /* Point to next descriptor */
            DmaTxDesc = (stc_eth_dma_desc_t *)(DmaTxDesc->u32Buf2NextDescAddr);
            /* Check if the buffer is available */
            if (0UL != (DmaTxDesc->u32ControlStatus & ETH_DMA_TXDESC_OWN)) {
                i32Ret = LL_ERR;
                goto error;
            }

            txBuffer = (uint8_t *)(DmaTxDesc->u32Buf1Addr);
            byteCnt = byteCnt - (ETH_TX_BUF_SIZE - bufferOffset);
            payloadOffset = payloadOffset + (ETH_TX_BUF_SIZE - bufferOffset);
            frameLength = frameLength + (ETH_TX_BUF_SIZE - bufferOffset);
            bufferOffset = 0UL;
        }
        /* Copy the remaining bytes */
        (void)memcpy((uint8_t *) & (txBuffer[bufferOffset]), (uint8_t *) & (((uint8_t *)q->payload)[payloadOffset]), byteCnt);
        bufferOffset = bufferOffset + byteCnt;
        frameLength = frameLength + byteCnt;
    }
    /* Prepare transmit descriptors to give to DMA */
    (void)ETH_DMA_SetTransFrame(&EthHandle, frameLength);
    i32Ret = LL_OK;

error:
    /* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
    if (RESET != ETH_DMA_GetStatus(ETH_DMA_FLAG_UNS)) {
        /* Clear DMA UNS flag */
        ETH_DMA_ClearStatus(ETH_DMA_FLAG_UNS);
        /* Resume DMA transmission */
        WRITE_REG32(CM_ETH->DMA_TXPOLLR, 0UL);
    }

    return i32Ret;
}

/**
 * @brief  Should allocate a pbuf and transfer the bytes of the incoming packet from the interface into the pbuf.
 * @param  None
 * @retval A pbuf filled with the received packet (including MAC header) or NULL on memory error.
 */
static struct pbuf *low_level_input(void)
{
    struct pbuf *p = NULL;
    struct pbuf *q;
    uint32_t len;
    uint8_t *rxBuffer;
    __IO stc_eth_dma_desc_t *DmaRxDesc;
    uint32_t byteCnt;
    uint32_t bufferOffset;
    uint32_t payloadOffset;
    uint32_t i;

    /* Get received frame */
    if (LL_OK != ETH_DMA_GetReceiveFrame(&EthHandle)) {
        return NULL;
    }

    /* Obtain the size of the packet */
    len = (EthHandle.stcRxFrame).u32Len;
    rxBuffer = (uint8_t *)(EthHandle.stcRxFrame).u32Buf;
    if (len > 0UL) {
        /* Allocate a pbuf chain of pbufs from the buffer */
        p = (struct pbuf *)malloc(sizeof(struct pbuf) + len);
        if (NULL != p) {
            p->next = NULL;
            p->payload = &((uint8_t *)p)[sizeof(struct pbuf)];
            p->len = len;
            (void)memset(p->payload, 0, p->len);
        }
    }
    if (p != NULL) {
        DmaRxDesc = (EthHandle.stcRxFrame).pstcFSDesc;
        bufferOffset = 0UL;
        for (q = p; q != NULL; q = q->next) {
            byteCnt = q->len;
            payloadOffset = 0UL;

            /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size */
            while ((byteCnt + bufferOffset) > ETH_RX_BUF_SIZE) {
                /* Copy data to pbuf */
                (void)memcpy((uint8_t *) & (((uint8_t *)q->payload)[payloadOffset]), (uint8_t *) & (rxBuffer[bufferOffset]), (ETH_RX_BUF_SIZE - bufferOffset));
                /* Point to next descriptor */
                DmaRxDesc = (stc_eth_dma_desc_t *)(DmaRxDesc->u32Buf2NextDescAddr);
                rxBuffer = (uint8_t *)(DmaRxDesc->u32Buf1Addr);
                byteCnt = byteCnt - (ETH_RX_BUF_SIZE - bufferOffset);
                payloadOffset = payloadOffset + (ETH_RX_BUF_SIZE - bufferOffset);
                bufferOffset = 0UL;
            }

            /* Copy remaining data in pbuf */
            (void)memcpy((uint8_t *) & (((uint8_t *)q->payload)[payloadOffset]), (uint8_t *) & (rxBuffer[bufferOffset]), byteCnt);
            bufferOffset = bufferOffset + byteCnt;
        }
    }
    /* Release descriptors to DMA */
    DmaRxDesc = (EthHandle.stcRxFrame).pstcFSDesc;
    for (i = 0UL; i < (EthHandle.stcRxFrame).u32SegCount; i++) {
        DmaRxDesc->u32ControlStatus |= ETH_DMA_RXDESC_OWN;
        DmaRxDesc = (stc_eth_dma_desc_t *)(DmaRxDesc->u32Buf2NextDescAddr);
    }
    /* Clear Segment_Count */
    (EthHandle.stcRxFrame).u32SegCount = 0UL;

    /* When Rx Buffer unavailable flag is set, clear it and resume reception */
    if (RESET != ETH_DMA_GetStatus(ETH_DMA_FLAG_RUS)) {
        /* Clear DMA RUS flag */
        ETH_DMA_ClearStatus(ETH_DMA_FLAG_RUS);
        /* Resume DMA reception */
        WRITE_REG32(CM_ETH->DMA_RXPOLLR, 0UL);
    }

    return p;
}

/**
 * @brief  Should be called at the beginning of the program to set up the network interface.
 * @param  None
 * @retval int32_t:
 *           - LL_OK: The IF is initialized
 *           - LL_ERR: The IF is uninitialized
 */
int32_t ethernetif_init(void)
{
    /* initialize the hardware */
    return low_level_init();
}

/**
 * @brief  This function should be called when a packet is ready to be read from the interface.
 * @param  None
 * @retval None
 */
void ethernetif_input(void)
{
    struct pbuf *p;

    /* Move received packet into a new pbuf */
    p = low_level_input();
    /* No packet could be read, silently ignore this */
    if (p != NULL) {
        /* Input callback */
        EthernetIF_InputCallback(p);
        free(p);
    }
}

/**
 * @brief  Input data handle callback.
 * @param  p                            Packets received.
 * @retval None
 */
__WEAKDEF void EthernetIF_InputCallback(struct pbuf *p)
{
    /* This is function could be implemented in user file when the callback is needed */
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
