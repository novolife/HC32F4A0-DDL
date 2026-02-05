/**
 *******************************************************************************
 * @file  cmp/cmp_normal_int/source/main.c
 * @brief Main program of CMP for the Device Driver Library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-10-31       CDT             Modify for driver update
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
 * @addtogroup CMP_Normal_Int
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

#define CMP_INT_SRC                     (INT_SRC_CMP1)
#define CMP_TEST_UNIT                   (CM_CMP1)
#define CMP_PERIP_CLK                   (FCG3_PERIPH_CMP1_2)
/* Define port and pin for CMP */
/* CMP1 compare voltage CMP1_INP4(CMP1_INP4) */
#define CMP1_INP4_PORT                  (GPIO_PORT_A)
#define CMP1_INP4_PIN                   (GPIO_PIN_03)
/* CMP1 reference voltage CMP1_INM4*/
#define CMP1_INM4_PORT                  (GPIO_PORT_C)
#define CMP1_INM4_PIN                   (GPIO_PIN_03)
/* CMP1_VCOUT1*/
#define CMP1_VCOUT1_PORT                (GPIO_PORT_B)
#define CMP1_VCOUT1_PIN                 (GPIO_PIN_12)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void CmpIrqCallback(void);
static void CmpConfig(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of cmp_normal_int project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_irq_signin_config_t stcIrqRegCfg;
    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_IO_Init();
    BSP_LED_Init();
    /* Configure CMP */
    CmpConfig();

    /*NVIC configuration for interrupt */
    stcIrqRegCfg.enIRQn = INT010_IRQn;
    stcIrqRegCfg.enIntSrc = CMP_INT_SRC;
    stcIrqRegCfg.pfnCallback = &CmpIrqCallback;
    (void)INTC_IrqSignIn(&stcIrqRegCfg);
    NVIC_ClearPendingIRQ(stcIrqRegCfg.enIRQn);
    NVIC_SetPriority(stcIrqRegCfg.enIRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(stcIrqRegCfg.enIRQn);
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);
    /* Configuration finished */
    for (;;) {
        ;
    }
}

/**
 * @brief  CMP interrupt call back
 * @param  None
 * @retval None
 */
static void CmpIrqCallback(void)
{
    if (SET == CMP_GetStatus(CMP_TEST_UNIT)) {
        BSP_LED_On(LED_RED);
        BSP_LED_Off(LED_BLUE);
    } else {
        BSP_LED_On(LED_BLUE);
        BSP_LED_Off(LED_RED);
    }
}

/**
 * @brief  Configure CMP.
 * @param  None
 * @retval None
 */
static void CmpConfig(void)
{
    stc_cmp_init_t stcCmpInit;
    stc_gpio_init_t stcGpioInit;

    /* Enable peripheral Clock */
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_CMBIAS, ENABLE);
    FCG_Fcg3PeriphClockCmd(CMP_PERIP_CLK, ENABLE);

    /* Port function configuration for CMP */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;
    (void)GPIO_Init(CMP1_INP4_PORT, CMP1_INP4_PIN, &stcGpioInit);
    (void)GPIO_Init(CMP1_INM4_PORT, CMP1_INM4_PIN, &stcGpioInit);
    GPIO_SetFunc(CMP1_VCOUT1_PORT, CMP1_VCOUT1_PIN, GPIO_FUNC_1);

    /* Clear structure */
    (void)CMP_StructInit(&stcCmpInit);
    stcCmpInit.u16PositiveInput = CMP1_POSITIVE_CMP1_INP4;
    stcCmpInit.u16NegativeInput = CMP_NEGATIVE_INM4;  /* CMP1_INM4 */
    stcCmpInit.u16OutPolarity = CMP_OUT_INVT_OFF;
    stcCmpInit.u16OutDetectEdge = CMP_DETECT_EDGS_BOTH;
    stcCmpInit.u16OutFilter = CMP_OUT_FILTER_CLK_DIV32;
    (void)CMP_NormalModeInit(CMP_TEST_UNIT, &stcCmpInit);

    /* Enable interrupt if need */
    CMP_IntCmd(CMP_TEST_UNIT, ENABLE);
    /* Enable CMP output */
    CMP_CompareOutCmd(CMP_TEST_UNIT, ENABLE);
    /* Enable VCOUT */
    CMP_PinVcoutCmd(CMP_TEST_UNIT, ENABLE);
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
