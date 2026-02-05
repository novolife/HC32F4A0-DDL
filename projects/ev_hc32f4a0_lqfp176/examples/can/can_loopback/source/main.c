/**
 *******************************************************************************
 * @file  can/can_loopback/source/main.c
 * @brief Main program of CAN loopback for the Device Driver Library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
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
 * @addtogroup CAN_Loopback
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* CAN FD function control. Non-zero to enable. */
#define CAN_FD_ENABLE                   (1U)

/* Unit definition of CAN in this example. */
#define CAN_SEL_UNIT1                   (0U)
#define CAN_SEL_UNIT2                   (1U)

/* Select a CAN unit. */
#if (CAN_FD_ENABLE > 0U)
/* CAN_UNIT_SEL can only be defined as CAN_SEL_UNIT2 */
#define CAN_UNIT_SEL                    (CAN_SEL_UNIT2)
#else
/* CAN_UNIT_SEL can be defined as CAN_SEL_UNIT1 or CAN_SEL_UNIT2 */
#define CAN_UNIT_SEL                    (CAN_SEL_UNIT1)
#endif

/* Definitions according to 'CAN_UNIT_SEL'. */
#if (CAN_UNIT_SEL == CAN_SEL_UNIT1)
#define CAN_UNIT                        (CM_CAN1)
#define CAN_PERIPH_CLK                  (FCG1_PERIPH_CAN1)

#define CAN_TX_PORT                     (GPIO_PORT_D)
#define CAN_TX_PIN                      (GPIO_PIN_05)
#define CAN_TX_PIN_FUNC                 (GPIO_FUNC_60)

#define CAN_RX_PORT                     (GPIO_PORT_D)
#define CAN_RX_PIN                      (GPIO_PIN_04)
#define CAN_RX_PIN_FUNC                 (GPIO_FUNC_61)

#define CAN_CLK_UNIT                    (CLK_CAN1)
#define CAN_CLK_SRC                     (CLK_CANCLK_SYSCLK_DIV3)

#elif (CAN_UNIT_SEL == CAN_SEL_UNIT2)
#define CAN_UNIT                        (CM_CAN2)
#define CAN_PERIPH_CLK                  (FCG1_PERIPH_CAN2)

#define CAN_TX_PORT                     (GPIO_PORT_D)
#define CAN_TX_PIN                      (GPIO_PIN_07)
#define CAN_TX_PIN_FUNC                 (GPIO_FUNC_62)

#define CAN_RX_PORT                     (GPIO_PORT_D)
#define CAN_RX_PIN                      (GPIO_PIN_06)
#define CAN_RX_PIN_FUNC                 (GPIO_FUNC_63)

#define CAN_CLK_UNIT                    (CLK_CAN2)
#define CAN_CLK_SRC                     (CLK_CANCLK_SYSCLK_DIV3)
#else
#error "The unit is NOT supported!!!"
#endif

/* Acceptance filter. */
#define CAN_FILTER_SEL                  (CAN_FILTER1)
#define CAN_FILTER_NUM                  (1U)

#define CAN_FILTER_ID                   (0UL)
#define CAN_FILTER_ID_MASK              (0x1FFFFFFFUL)
#define CAN_FILTER_ID_TYPE              (CAN_ID_STD_EXT)

/* Message ID definitions. */
#define CAN_TX_ID1                      (0xA1UL)
#define CAN_TX_ID1_IDE                  (0U)

#define CAN_TX_ID2                      (0xB2UL)
#define CAN_TX_ID2_IDE                  (1U)

#define CAN_TX_ID3                      (0xC3UL)
#define CAN_TX_ID3_IDE                  (1U)

/* Size of the TX data. */
#if (CAN_FD_ENABLE > 0U)
#define CAN_TX_DLC                      (CAN_DLC64)
#define CAN_TX_DATA_SIZE                (64U)
#else
#define CAN_TX_DLC                      (CAN_DLC8)
#define CAN_TX_DATA_SIZE                (8U)
#endif

/* Max size of the RX data. */
#define CAN_RX_DATA_SIZE_MAX            (CAN_TX_DATA_SIZE)

/* Number of RX frame */
#define CAN_RX_FRAME_NUM                (8U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void CanConfig(void);
static void CanPhyEnable(void);

static void CanTx(void);
static void CanRx(void);
static int32_t CanLoopbackVerify(const uint8_t *puExpectData,
                                 uint8_t *pu8RxData,
                                 uint8_t u8Size);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
#if (LL_PRINT_ENABLE == DDL_ON)
const static char *m_s8IDTypeStr[] = {
    "standard",
    "extended",
};

const static char *m_s8FrameTypeStr[] = {
    "classical",
    "CAN-FD",
};

const static uint8_t m_au8DLC2Size[16U] = {
    0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 12U, 16U, 20U, 24U, 32U, 48U, 64U
};
#endif

static stc_can_rx_frame_t m_astRxFrame[CAN_RX_FRAME_NUM];

static stc_can_tx_frame_t m_stcTx1;
static stc_can_tx_frame_t m_stcTx2;
static stc_can_tx_frame_t m_stcTx3;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Main function of can_loopback project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);
    /* Configures the system clock to 240MHz. */
    BSP_CLK_Init();
    /* Initializes UART for debug printing. Baudrate is 115200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configures CAN. */
    CanConfig();
    /* Set CAN PHY STB pin as low. */
    CanPhyEnable();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /***************** Configuration end, application start **************/
    for (;;) {
        CanTx();
        CanRx();
        DDL_DelayMS(500U);
    }
}

/**
 * @brief  CAN configuration.
 * @param  None
 * @retval None
 */
static void CanConfig(void)
{
    stc_can_init_t stcCanInit;
    stc_can_filter_config_t astcFilter[CAN_FILTER_NUM] = {
        {.u32ID = CAN_FILTER_ID, .u32IDMask = CAN_FILTER_ID_MASK, .u32IDType = CAN_FILTER_ID_TYPE},
    };
#if (CAN_FD_ENABLE > 0U)
    stc_canfd_config_t stcCanFd;
#endif

    /* Set the function of CAN pins. */
    GPIO_SetFunc(CAN_TX_PORT, CAN_TX_PIN, CAN_TX_PIN_FUNC);
    GPIO_SetFunc(CAN_RX_PORT, CAN_RX_PIN, CAN_RX_PIN_FUNC);

    /* Configures CAN clock. */
    CLK_SetCANClockSrc(CAN_CLK_UNIT, CAN_CLK_SRC);

#if (CAN_FD_ENABLE == 0U)
    /* Initializes CAN. */
    (void)CAN_StructInit(&stcCanInit);
    stcCanInit.stcBitCfg.u32Prescaler = 4U;
    stcCanInit.stcBitCfg.u32TimeSeg1  = 15U;
    stcCanInit.stcBitCfg.u32TimeSeg2  = 5U;
    stcCanInit.stcBitCfg.u32SJW       = 5U;
    stcCanInit.pstcFilter             = astcFilter;
    stcCanInit.u16FilterSelect        = CAN_FILTER_SEL;
    stcCanInit.u8WorkMode             = CAN_WORK_MD_ELB;
    stcCanInit.u8SelfAck              = CAN_SELF_ACK_ENABLE;
#else
    /* Initializes CAN. */
    (void)CAN_StructInit(&stcCanInit);
    stcCanInit.stcBitCfg.u32Prescaler = 1U;
    stcCanInit.stcBitCfg.u32TimeSeg1  = 64U;
    stcCanInit.stcBitCfg.u32TimeSeg2  = 16U;
    stcCanInit.stcBitCfg.u32SJW       = 16U;
    stcCanInit.pstcFilter             = astcFilter;
    stcCanInit.u16FilterSelect        = CAN_FILTER_SEL;
    stcCanInit.u8WorkMode             = CAN_WORK_MD_ELB;
    stcCanInit.u8SelfAck              = CAN_SELF_ACK_ENABLE;

    /* CAN-FD configuration. */
    (void)CAN_FD_StructInit(&stcCanFd);
    stcCanFd.stcBitCfg.u32Prescaler = 1U;
    stcCanFd.stcBitCfg.u32TimeSeg1  = 8U;
    stcCanFd.stcBitCfg.u32TimeSeg2  = 2U;
    stcCanFd.stcBitCfg.u32SJW       = 2U;
    stcCanFd.u8TDC                  = CAN_FD_TDC_ENABLE;
    stcCanFd.u8SSPOffset            = stcCanFd.stcBitCfg.u32TimeSeg1;
    stcCanInit.pstcCanFd = &stcCanFd;
#endif

    FCG_Fcg1PeriphClockCmd(CAN_PERIPH_CLK, ENABLE);
    (void)CAN_Init(CAN_UNIT, &stcCanInit);
    /* Enable the interrupts, the status flags can be read. */
    CAN_IntCmd(CAN_UNIT, CAN_INT_ALL, ENABLE);
}

/**
 * @brief  Set CAN PHY STB pin as low.
 * @param  None
 * @retval None
 */
static void CanPhyEnable(void)
{
    BSP_IO_Init();
    BSP_CAN_STB_IO_Init();

    /* Set PHY STB pin as low. */
    BSP_CAN_STBCmd(EIO_PIN_RESET);
}

/**
 * @brief  CAN transmission.
 * @param  None
 * @retval None
 */
static void CanTx(void)
{
    uint8_t i;
    static uint8_t u8Data;

    for (i = 0U; i < CAN_TX_DATA_SIZE; i++) {
        m_stcTx1.au8Data[i] = u8Data++;
        m_stcTx2.au8Data[i] = u8Data++;
        m_stcTx3.au8Data[i] = u8Data++;
    }

    /* Frame with CAN_ID1 */
    m_stcTx1.u32Ctrl = 0x0UL;
    m_stcTx1.u32ID   = CAN_TX_ID1;
    m_stcTx1.IDE     = CAN_TX_ID1_IDE;
    m_stcTx1.DLC     = CAN_TX_DLC;
#if (CAN_FD_ENABLE > 0U)
    m_stcTx1.FDF   = 1U;
    m_stcTx1.BRS   = 1U;
#endif
    (void)CAN_FillTxFrame(CAN_UNIT, CAN_TX_BUF_PTB, &m_stcTx1);
    /* Start PTB TX */
    CAN_StartTx(CAN_UNIT, CAN_TX_REQ_PTB);
    /* Check transmission end. */
    while (CAN_GetStatus(CAN_UNIT, CAN_FLAG_PTB_TX) == RESET) { }
    CAN_ClearStatus(CAN_UNIT, CAN_FLAG_PTB_TX);

    /* Frame with CAN_ID2 */
    m_stcTx2.u32Ctrl = 0x0UL;
    m_stcTx2.u32ID   = CAN_TX_ID2;
    m_stcTx2.IDE     = CAN_TX_ID2_IDE;
    m_stcTx2.DLC     = CAN_TX_DLC;
#if (CAN_FD_ENABLE > 0U)
    m_stcTx2.FDF   = 1U;
    m_stcTx2.BRS   = 1U;
#endif
    (void)CAN_FillTxFrame(CAN_UNIT, CAN_TX_BUF_STB, &m_stcTx2);

    /* Frame with CAN_ID3 */
    m_stcTx3.u32Ctrl = 0x0UL;
    m_stcTx3.u32ID   = CAN_TX_ID3;
    m_stcTx3.IDE     = CAN_TX_ID3_IDE;
    m_stcTx3.DLC     = CAN_TX_DLC;
#if (CAN_FD_ENABLE > 0U)
    m_stcTx3.FDF   = 1U;
    m_stcTx3.BRS   = 1U;
#endif
    (void)CAN_FillTxFrame(CAN_UNIT, CAN_TX_BUF_STB, &m_stcTx3);

    /* Start STB TX */
    CAN_StartTx(CAN_UNIT, CAN_TX_REQ_STB_ALL);
    /* Check transmission end. */
    while (CAN_GetStatus(CAN_UNIT, CAN_FLAG_STB_TX) == RESET) { }
    CAN_ClearStatus(CAN_UNIT, CAN_FLAG_STB_TX);
}

/**
 * @brief  CAN receives data.
 * @param  None
 * @retval None
 */
static void CanRx(void)
{
    uint8_t i;
    uint8_t j;
    uint8_t u8RxDataSize;
    uint8_t u8RxFrameNum = 0U;
    int32_t i32Ret;

    /* Get all received frames. */
    do {
        i32Ret = CAN_GetRxFrame(CAN_UNIT, &m_astRxFrame[u8RxFrameNum]);
        if (i32Ret == LL_OK) {
            u8RxFrameNum++;
        }
    } while (i32Ret == LL_OK);

    /* Handle received frames. */
    for (i = 0U; i < u8RxFrameNum; i++) {
        DDL_Printf("--------------------------------------\r\n");
        DDL_Printf("CAN received %s frame with %s ID %.8x:\r\n", \
                   m_s8FrameTypeStr[m_astRxFrame[i].FDF], \
                   m_s8IDTypeStr[m_astRxFrame[i].IDE],    \
                   (unsigned int)m_astRxFrame[i].u32ID);
        u8RxDataSize = m_au8DLC2Size[m_astRxFrame[i].DLC];
        for (j = 0; j < u8RxDataSize; j++) {
            DDL_Printf(" %.2x.", m_astRxFrame[i].au8Data[j]);
        }
        DDL_Printf("\r\n");

        if (m_astRxFrame[i].TX == 1U) {
            DDL_Printf("This is a self-transmitted frame.");
            switch (m_astRxFrame[i].u32ID) {
                case CAN_TX_ID1:
                    i32Ret = CanLoopbackVerify(m_stcTx1.au8Data, m_astRxFrame[i].au8Data, u8RxDataSize);
                    break;
                case CAN_TX_ID2:
                    i32Ret = CanLoopbackVerify(m_stcTx2.au8Data, m_astRxFrame[i].au8Data, u8RxDataSize);
                    break;
                case CAN_TX_ID3:
                    i32Ret = CanLoopbackVerify(m_stcTx3.au8Data, m_astRxFrame[i].au8Data, u8RxDataSize);
                    break;
                default:
                    i32Ret = LL_ERR;
                    break;
            }

            if (i32Ret == LL_OK) {
                DDL_Printf("CAN loopback OK.\r\n");
            } else {
                DDL_Printf("CAN loopback FAIL!!!\r\n");
                for (;;) {
                    /* rsvd */
                }
            }
        }
    }
}

/**
 * @brief  Verify the received data.
 * @param  [in] puExpectData            Pointer to the expect data buffer.
 * @param  [in] pu8RxData               Pointer to the received data buffer.
 * @param  [in] u8Size                  Verification size.
 * @retval int32_t:
 *         - LL_OK:                     Verification OK.
 *         - LL_ERR:                    Verification FAIL.
 */
static int32_t CanLoopbackVerify(const uint8_t *puExpectData,
                                 uint8_t *pu8RxData,
                                 uint8_t u8Size)
{
    uint8_t i;
    int32_t i32Ret = LL_OK;

    for (i = 0U; i < u8Size; i++) {
        if (pu8RxData[i] != puExpectData[i]) {
            i32Ret = LL_ERR;
            break;
        }
        pu8RxData[i] = 0U;
    }

    return i32Ret;
}

/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
