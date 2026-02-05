/**
 *******************************************************************************
 * @file  eth/eth_twoboards/source/main.c
 * @brief This example code realizes the function of ethernet communication
 *        between two boards.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-10-31       CDT             Add configuration of XTAL IO as analog function
   2023-09-30       CDT             SysTick_Handler add __DSB for Arm Errata 838869
   2024-11-08       CDT             Re-config SRAM read/write wait cycle
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
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

/* Clock output Port/Pin definition */
#define ETH_CLK_MCO_PORT                (GPIO_PORT_F)
#define ETH_CLK_MCO_PIN                 (GPIO_PIN_00)
#define ETH_CLK_MCO_CH                  (CLK_MCO1)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
/* eth tx buffer */
static struct pbuf txPbuf;
static char txBuf[] = "Customers should select appropriate products for your \
                       application, and design, verify and test your application.";

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
static void ETH_ClockOutputConfig(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    (void)GPIO_Init(ETH_CLK_MCO_PORT, ETH_CLK_MCO_PIN, &stcGpioInit);
    GPIO_SetFunc(ETH_CLK_MCO_PORT, ETH_CLK_MCO_PIN, GPIO_FUNC_1);
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
    /* Configure clock output */
    ETH_ClockOutputConfig();
    /* Configure UART */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* SysTick configuration */
    (void)SysTick_Init(1000U);
    /* Set Systick Interrupt to the highest priority */
    NVIC_SetPriority(SysTick_IRQn, DDL_IRQ_PRIO_00);
}

/**
 * @brief  Input data handle callback.
 * @param  p                            Packets received.
 * @retval None
 */
void EthernetIF_InputCallback(struct pbuf *p)
{
    /* txPbuf == p*/
    if ((0 == (memcmp(p->payload, txPbuf.payload, p->len))) && (p->len == txPbuf.len)) {
        BSP_LED_On(LED_BLUE);
        BSP_LED_Off(LED_RED);
    } else {
        BSP_LED_On(LED_RED);
        BSP_LED_Off(LED_BLUE);
    }
}

/**
 * @brief  Main function of ETH Two Boards.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_Config();
    /* Configure the Ethernet */
    (void)ethernetif_init();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);
    /* fill data to txPbuf */
    txPbuf.next    = NULL;
    txPbuf.payload = txBuf;
    txPbuf.len     = strlen(txBuf);

    for (;;) {
        if (SET == BSP_KEY_GetStatus(BSP_KEY_2)) {
            if (LL_OK != low_level_output(&txPbuf)) {
                /* Send data error! */
                BSP_LED_On(LED_YELLOW);
            } else {
                BSP_LED_Off(LED_YELLOW);
            }
        }
        /* Read a received packet */
        (void)ethernetif_input();
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
