/**
 *******************************************************************************
 * @file  cmp/cmp_window/source/main.c
 * @brief Main program of CMP for the Device Driver Library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-10-31       CDT             Modify for driver update
   2024-11-08       CDT             Delete DAC_DataRegAlignConfig() due to align configure in DAC_Init()
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
 * @addtogroup CMP_Window
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

#define DAC_TEST_UNIT                   (CM_DAC2)
#define DAC_PERIP_CLK                   (FCG3_PERIPH_DAC2)

#define CMP_PERIP_CLK                   (FCG3_PERIPH_CMP3_4)
/* Define port and pin of CMP */
/* CMP4 compare voltage CMP4_INP3 */
#define CMP4_INP3_PORT                  (GPIO_PORT_E)
#define CMP4_INP3_PIN                   (GPIO_PIN_14)
/* CMP3 reference low voltage CMP3_INM1(DAC2_OUT1)*/
#define CMP3_INM1_PORT                  (GPIO_PORT_C)
#define CMP3_INM1_PIN                   (GPIO_PIN_04)
/* CMP4 reference high voltage CMP4_INM2(DAC2_OUT2)*/
#define CMP4_INM2_PORT                  (GPIO_PORT_C)
#define CMP4_INM2_PIN                   (GPIO_PIN_05)
/* CMP4_VCOUT4*/
#define CMP4_VCOUT4_PORT                (GPIO_PORT_D)
#define CMP4_VCOUT4_PIN                 (GPIO_PIN_02)

#define DAC_VOL_1P3V                    (0x1000U/33U*13U)
#define DAC_VOL_2V                      (0x1000U/33U*20U)
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Configure CMP.
 * @param  None
 * @retval None
 */
static void CmpConfig(void)
{
    stc_cmp_window_init_t stcWindowModeInit;
    stc_gpio_init_t stcGpioInit;
    /* Enable peripheral Clock */
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_CMBIAS, ENABLE);
    FCG_Fcg3PeriphClockCmd(CMP_PERIP_CLK, ENABLE);
    /* Port function configuration for CMP*/
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;
    (void)GPIO_Init(CMP4_INP3_PORT, CMP4_INP3_PIN, &stcGpioInit); /* CMP compare voltage */
    (void)GPIO_Init(CMP4_INM2_PORT, CMP4_INM2_PIN, &stcGpioInit); /* CMP4 reference high voltage */
    (void)GPIO_Init(CMP3_INM1_PORT, CMP3_INM1_PIN, &stcGpioInit); /* CMP3 reference low voltage */
    GPIO_SetFunc(CMP4_VCOUT4_PORT, CMP4_VCOUT4_PIN, GPIO_FUNC_1); /* CMP4_VCOUT4 */

    /* Configuration for normal compare function */
    CMP_WindowStructInit(&stcWindowModeInit);
    stcWindowModeInit.u16WinVolLow = CMP_WIN_LOW_INM1;  /* DAC2_OUT1 */
    stcWindowModeInit.u16WinVolHigh = CMP_WIN_HIGH_INM2;  /* DAC2_OUT2 */
    stcWindowModeInit.u16OutPolarity = CMP_OUT_INVT_OFF;
    stcWindowModeInit.u16OutDetectEdge = CMP_DETECT_EDGS_BOTH;
    stcWindowModeInit.u16OutFilter = CMP_OUT_FILTER_CLK_DIV32;
    (void)CMP_WindowModeInit(CMP_WIN_CMP34, &stcWindowModeInit);

    /* Enable CMP output */
    CMP_CompareOutCmd(CM_CMP3, ENABLE);
    CMP_CompareOutCmd(CM_CMP4, ENABLE);

    /* Enable VCOUT */
    CMP_PinVcoutCmd(CM_CMP4, ENABLE);
}

/**
 * @brief  Configure DAC.
 * @param  None
 * @retval None
 */
static void DacConfig(void)
{
    stc_dac_init_t stcDacInit;
    /* Enable peripheral Clock */
    FCG_Fcg3PeriphClockCmd(DAC_PERIP_CLK, ENABLE);

    DAC_DeInit(DAC_TEST_UNIT);
    (void)DAC_StructInit(&stcDacInit);
    (void)DAC_Init(DAC_TEST_UNIT, DAC_CH1, &stcDacInit);
    (void)DAC_Init(DAC_TEST_UNIT, DAC_CH2, &stcDacInit);

    /* Clear data register */
    DAC_SetChData(DAC_TEST_UNIT, DAC_CH1, 0U);
    DAC_SetChData(DAC_TEST_UNIT, DAC_CH2, 0U);

    (void)DAC_AMPCmd(DAC_TEST_UNIT, DAC_CH1, ENABLE);
    (void)DAC_AMPCmd(DAC_TEST_UNIT, DAC_CH2, ENABLE);

    /* Output Enable */
    DAC_StartDualCh(DAC_TEST_UNIT);

    /* Write Data  V = (Conversion Data / 4096) * VREFH */
    DAC_SetChData(DAC_TEST_UNIT, DAC_CH1, DAC_VOL_1P3V);
    DAC_SetChData(DAC_TEST_UNIT, DAC_CH2, DAC_VOL_2V);
}

/**
 * @brief  Main function of cmp_window project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);

    BSP_CLK_Init();
    BSP_IO_Init();
    /*DAC_UNIT2_CH1_PIN(PC04) and DAC_UNIT2_CH2_PIN(PC05) are also used by
    * Ethernet PHY module on BSP_EV_HC32F4A0_LQFP176 board. Pull down reset pin
    * of Ethernet to eliminate the influence from Ethernet
    */
    BSP_IO_ConfigPortPin(EIO_PORT1, EIO_ETH_RST, EIO_DIR_OUT);
    BSP_IO_WritePortPin(EIO_PORT1, EIO_ETH_RST, (uint8_t)DISABLE);

    /* Configure DAC */
    DacConfig();
    /* Configure CMP */
    CmpConfig();
    /* Lock peripherals or registers */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);
    for (;;) {
        ;
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
