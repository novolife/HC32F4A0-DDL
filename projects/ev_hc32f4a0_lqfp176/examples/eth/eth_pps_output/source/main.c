/**
 *******************************************************************************
 * @file  eth/eth_pps_output/source/main.c
 * @brief This example code implements a ethernet loopback function.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2023-09-30       CDT             First version
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
 * @addtogroup ETH_Pps_Output
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

/* ETH PPT definition */
#define ETH_PTP_SUBSEC_ADDEND           (43U)               /* 20ns */
#define ETH_PTP_CALC_DIVISOR            (0x100000000UL)     /* 2^32 */
#define ETH_PTP_BASE_FREQ               (50U * 1000U * 1000U)

/* Clock output Port/Pin definition */
#define ETH_CLK_MCO_PORT1               (GPIO_PORT_F)
#define ETH_CLK_MCO_PIN1                (GPIO_PIN_00)
#define ETH_CLK_MCO_PORT2               (GPIO_PORT_E)
#define ETH_CLK_MCO_PIN2                (GPIO_PIN_00)
#define ETH_CLK_MCO_FUNC                (GPIO_FUNC_1)
#define ETH_CLK_MCO_CH                  (CLK_MCO1)

/* ETH PPS definition */
#define ETH_PPS_OUT_PORT                (GPIO_PORT_G)
#define ETH_PPS_OUT_PIN                 (GPIO_PIN_08)
#define ETH_PPS_OUT_FUNC                (GPIO_FUNC_11)
#define ETH_PPS_OUT_MD                  (ETH_PPS_OUTPUT_MD_ONCE)
#define ETH_PPS_OUT_FREQ                (ETH_PPS_OUTPUT_ONE_PULSE)
#define ETH_PPS_CH                      (ETH_PPS_CH0)

#define ETH_PPS_IRQn                    (INT001_IRQn)
#define ETH_PPS_INT_SRC                 (INT_SRC_ETH_GLB_INT)

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
__IO static uint32_t u32IntCnt = 0U;
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
 * @brief  SysTick interrupt callback function.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
    SysTick_IncTick();
    __DSB();  /* Arm Errata 838869 */
}

/**
 * @brief  ETH Clock output configuration.
 * @param  None
 * @retval None
 */
static void Ethernet_ClockOutputConfig(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    (void)GPIO_Init(ETH_CLK_MCO_PORT1, ETH_CLK_MCO_PIN1, &stcGpioInit);
    (void)GPIO_Init(ETH_CLK_MCO_PORT2, ETH_CLK_MCO_PIN2, &stcGpioInit);
    GPIO_SetFunc(ETH_CLK_MCO_PORT1, ETH_CLK_MCO_PIN1, ETH_CLK_MCO_FUNC);
    GPIO_SetFunc(ETH_CLK_MCO_PORT2, ETH_CLK_MCO_PIN2, ETH_CLK_MCO_FUNC);
    /* Configure clock output system clock = 25Mhz */
    CLK_MCOConfig(ETH_CLK_MCO_CH, CLK_MCO_SRC_PLLP, CLK_MCO_DIV8);
    /* MCO output enable */
    CLK_MCOCmd(ETH_CLK_MCO_CH, ENABLE);
}

/**
 * @brief  BSP clock initialize.
 *         SET board system clock to PLL@200MHz
 * @param  None
 * @retval None
 */
void BSP_CLK_Init(void)
{
    stc_clock_xtal_init_t stcXtalInit;
    stc_clock_pll_init_t stcPLLHInit;

    CLK_SetClockDiv(CLK_BUS_CLK_ALL,
                    (CLK_PCLK0_DIV1 | CLK_PCLK1_DIV2 | CLK_PCLK2_DIV4 |
                     CLK_PCLK3_DIV4 | CLK_PCLK4_DIV2 | CLK_EXCLK_DIV2 |
                     CLK_HCLK_DIV1));
    (void)CLK_XtalStructInit(&stcXtalInit);
    /* Config Xtal and enable Xtal */
    stcXtalInit.u8Mode   = CLK_XTAL_MD_OSC;
    stcXtalInit.u8Drv    = CLK_XTAL_DRV_ULOW;
    stcXtalInit.u8State  = CLK_XTAL_ON;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_2MS;

    GPIO_AnalogCmd(BSP_XTAL_PORT, BSP_XTAL_PIN, ENABLE);
    (void)CLK_XtalInit(&stcXtalInit);

    (void)CLK_PLLStructInit(&stcPLLHInit);
    /* VCO = (8/1)*100 = 800MHz*/
    stcPLLHInit.u8PLLState      = CLK_PLL_ON;
    stcPLLHInit.PLLCFGR         = 0UL;
    stcPLLHInit.PLLCFGR_f.PLLM  = 1UL - 1UL;
    stcPLLHInit.PLLCFGR_f.PLLN  = 100UL - 1UL;
    stcPLLHInit.PLLCFGR_f.PLLP  = 4UL - 1UL;
    stcPLLHInit.PLLCFGR_f.PLLQ  = 4UL - 1UL;
    stcPLLHInit.PLLCFGR_f.PLLR  = 4UL - 1UL;
    stcPLLHInit.PLLCFGR_f.PLLSRC = CLK_PLL_SRC_XTAL;
    (void)CLK_PLLInit(&stcPLLHInit);

    /* SRAM Read/Write wait cycle setting */
    SRAM_SetWaitCycle(SRAM_SRAMH, SRAM_WAIT_CYCLE0, SRAM_WAIT_CYCLE0);
    SRAM_SetWaitCycle((SRAM_SRAM123 | SRAM_SRAM4 | SRAM_SRAMB), SRAM_WAIT_CYCLE1, SRAM_WAIT_CYCLE1);
    /* 0-wait @ 40MHz */
    EFM_SetWaitCycle(EFM_WAIT_CYCLE5);
    /* 4 cycles for 200 ~ 250MHz */
    GPIO_SetReadWaitCycle(GPIO_RD_WAIT4);
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_PLL);
}

/**
 * @brief  Configure the BSP.
 * @param  None
 * @retval None
 */
static void BSP_Config(void)
{
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_IO_Init();
    BSP_LED_Init();
    BSP_KEY_Init();
    /* OutPut clock(MCO) to RX_CLK/TX_CLK */
    Ethernet_ClockOutputConfig();
    /* Configure UART */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* SysTick configuration */
    (void)SysTick_Init(1000U);
    /* Set Systick Interrupt to the highest priority */
    NVIC_SetPriority(SysTick_IRQn, DDL_IRQ_PRIO_00);
}

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
    /* Hold reset phy */
    BSP_IO_ConfigPortPin(EIO_PORT1, EIO_ETH_RST, EIO_DIR_OUT);
    BSP_IO_WritePortPin(EIO_PORT1, EIO_ETH_RST, EIO_PIN_RESET);
    SysTick_Delay(PHY_HW_RST_DELAY);

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
 * @brief  Initialize the Ethernet PTP.
 * @param  None
 * @retval None
 * @
 */
void Ethernet_PtpInit(void)
{
    stc_eth_ptp_init_t stcEthPtpInit;
    uint32_t u32Addend, u32Pclk1;
    uint16_t u16Cnt;

    u32Pclk1 = CLK_GetBusClockFreq(CLK_BUS_PCLK1);
    u32Addend = (uint32_t)((uint64_t)ETH_PTP_CALC_DIVISOR / (u32Pclk1 / ETH_PTP_BASE_FREQ));
    /* Configure timestamp */
    for (u16Cnt = 0; u16Cnt < ETH_TX_BUF_NUM; u16Cnt++) {
        ETH_DMA_TxDescTimestamp(&EthDmaTxDscrTab[u16Cnt], ENABLE);
    }
    ETH_MAC_IntCmd(ETH_MAC_INT_TSPIM, DISABLE);
    ETH_PTP_StructInit(&stcEthPtpInit);
    stcEthPtpInit.u32SnapFrameType   = ETH_PTP_FRAME_TYPE_RX_FRAME;
    stcEthPtpInit.u32CalibMode       = ETH_PTP_CALIB_MD_FINE;
    stcEthPtpInit.u32BasicAddend     = u32Addend;
    stcEthPtpInit.u8SubsecAddend     = ETH_PTP_SUBSEC_ADDEND;
    stcEthPtpInit.u32SecInitValue    = 0x100U;
    stcEthPtpInit.u32SubsecInitValue = 0x10U;
    ETH_PTP_Init(&stcEthPtpInit);
}

/**
 * @brief  Initialize the Ethernet.
 * @param  None
 * @retval None
 */
static void Ethernet_init(void)
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
    stcEthInit.stcDmaInit.u32RxBurstLen             = ETH_DMA_RX_BURST_LEN_1BEAT;
    stcEthInit.stcDmaInit.u32TxBurstLen             = ETH_DMA_TX_BURST_LEN_1BEAT;
    stcEthInit.stcDmaInit.u32DropChecksumErrorFrame = ETH_DMA_DROP_CHECKSUM_ERR_FRAME_DISABLE;
    stcEthInit.stcDmaInit.u32ForwardErrorFrame      = ETH_DMA_FORWARD_ERR_FRAME_ENABLE;
    stcEthInit.stcDmaInit.u32ForwardUndersizeFrame  = ETH_DMA_FORWARD_UNDERSIZE_FRAME_ENABLE;
    /* MAC */
    stcEthInit.stcMacInit.u32AutoStripPadFCS    = ETH_MAC_AUTO_STRIP_PAD_FCS_ENABLE;
    stcEthInit.stcMacInit.u32ReceiveOwn         = ETH_MAC_RX_OWN_DISABLE;
    stcEthInit.stcMacInit.u32ReceiveAll         = ETH_MAC_RX_ALL_ENABLE;
    stcEthInit.stcMacInit.u32PassControlFrame   = ETH_MAC_PASS_CTRL_FRAME_FORWARD_ALL;

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
    /* Initialize PTP */
    Ethernet_PtpInit();
    /* Set Mac loopback mode */
    ETH_MAC_LoopBackCmd(ENABLE);
    /* Enable MAC and DMA transmission and reception */
    (void)ETH_Start();
}

/**
 * @brief  Initialize ETH PPS.
 * @param  None
 * @retval None
 */
static void Ethernet_PpsInit(void)
{
    stc_eth_pps_config_t stcEthPpsInit;

    GPIO_SetFunc(ETH_PPS_OUT_PORT, ETH_PPS_OUT_PIN, ETH_PPS_OUT_FUNC);

    ETH_PPS_StructInit(&stcEthPpsInit);
    stcEthPpsInit.u32OutputMode  = ETH_PPS_OUT_MD;
    stcEthPpsInit.u32OutputFreq  = ETH_PPS_OUT_FREQ;
    stcEthPpsInit.u32TriggerFunc = ETH_PPS_TRIG_FUNC_INT_PPS_EVT;
    stcEthPpsInit.u32SecValue    = 0x105U;
    stcEthPpsInit.u32SubsecValue = 0x1000000U;
    ETH_PPS_Init(ETH_PPS_CH, &stcEthPpsInit);

    ETH_PPS_StructInit(&stcEthPpsInit);
    stcEthPpsInit.u32TriggerFunc = ETH_PPS_TRIG_FUNC_PPS_EVT;
    stcEthPpsInit.u32SecValue    = 0x1000000U;
    ETH_PPS_Init(ETH_PPS_CH1, &stcEthPpsInit);
}

/**
 * @brief  ETH PPS irq callback.
 * @param  None
 * @retval None
 */
void ETH_PPS_IrqCallback(void)
{
    u32IntCnt++;
    if (SET == ETH_PTP_GetStatus(ETH_PTP_FLAG_TSTAR0)) {
        DDL_Printf("The PPS0 occurs interrupt.\r\n");
    }
    DDL_Printf("The PTP interrupt has occurred %lu times.\r\n\r\n", u32IntCnt);
}

/**
 * @brief  Initialize ETH PPS interrupt.
 * @param  None
 * @retval None
 */
static void Ethernet_PpsIntInit(void)
{
    stc_irq_signin_config_t stcIrqSignConfig;

    /* PPS IRQ sign-in */
    stcIrqSignConfig.enIntSrc    = ETH_PPS_INT_SRC;
    stcIrqSignConfig.enIRQn      = ETH_PPS_IRQn;
    stcIrqSignConfig.pfnCallback = &ETH_PPS_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(ETH_PPS_IRQn);
    NVIC_SetPriority(ETH_PPS_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(ETH_PPS_IRQn);

    /* Enable PTP interrupt */
    ETH_MAC_IntCmd(ETH_MAC_INT_TSPIM, ENABLE);
    ETH_PTP_IntCmd(ENABLE);
}

/**
 * @brief  Main function of ETH Loopback.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint32_t u32Sec, u32SubSec;

    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_Config();
    /* Init the Ethernet */
    Ethernet_init();
    /* Init the PPS */
    Ethernet_PpsInit();
    /* Init the PPS interrupt */
    Ethernet_PpsIntInit();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        if (SET == BSP_KEY_GetStatus(BSP_KEY_1)) {
            ETH_PTP_GetSysTime(&u32Sec, &u32SubSec);
            ETH_PPS_SetTargetTime(ETH_PPS_CH, u32Sec + 0x02U, 0x1000000U);
            ETH_PPS_SetPpsOutputFreq(ETH_PPS_CH, ETH_PPS_OUT_FREQ);
            ETH_PTP_IntCmd(ENABLE);
        }
        DDL_DelayMS(500U);
    }
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
