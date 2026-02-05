/**
 *******************************************************************************
 * @file  cmp/cmp_normal_blankwindow/source/main.c
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
 * @addtogroup CMP_Normal_TimerWindows
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

#define TMR6_UNIT                       (CM_TMR6_1)
#define TMR6_PERIP_CLK                  (FCG2_PERIPH_TMR6_1)
#define TMR6_PERIOD_VAL                 (60000UL)
#define TMR6_CMP_VAL                    (30000UL)

#define TMR6_1_PWMA_PORT                (GPIO_PORT_B)
#define TMR6_1_PWMA_PIN                 (GPIO_PIN_09)
/* select TMR6_1_PWMA as window signal of CMP1 */
#define CMP_BW_PWM                      (CMP_BLANKWIN_SRC7)

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
    stc_cmp_init_t stcCmpInit;
    stc_gpio_init_t stcGpioInit;
    stc_cmp_blankwindow_t stcBlankWindowInit;

    /* Enable peripheral Clock */
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_CMBIAS, ENABLE);
    FCG_Fcg3PeriphClockCmd(CMP_PERIP_CLK, ENABLE);

    /* Port function configuration for CMP */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;
    (void)GPIO_Init(CMP1_INP4_PORT, CMP1_INP4_PIN, &stcGpioInit);
    (void) GPIO_Init(CMP1_INM4_PORT, CMP1_INM4_PIN, &stcGpioInit);
    GPIO_SetFunc(CMP1_VCOUT1_PORT, CMP1_VCOUT1_PIN, GPIO_FUNC_1);

    /* Clear structure */
    (void)CMP_StructInit(&stcCmpInit);
    stcCmpInit.u16PositiveInput = CMP1_POSITIVE_CMP1_INP4;
    stcCmpInit.u16NegativeInput = CMP_NEGATIVE_INM4;  /* CMP1_INM4 */
    stcCmpInit.u16OutPolarity = CMP_OUT_INVT_OFF;
    stcCmpInit.u16OutDetectEdge = CMP_DETECT_EDGS_BOTH;
    stcCmpInit.u16OutFilter = CMP_OUT_FILTER_CLK_DIV32;
    (void)CMP_NormalModeInit(CMP_TEST_UNIT, &stcCmpInit);

    stcBlankWindowInit.u16Src = CMP_BW_PWM;
    stcBlankWindowInit.u8ValidLevel = CMP_BLANKWIN_VALID_LVL_HIGH;
    stcBlankWindowInit.u8OutLevel = CMP_BLANKWIN_OUTPUT_LVL_HIGH;
    (void)CMP_BlankWindowConfig(CMP_TEST_UNIT, &stcBlankWindowInit);
    CMP_BlankWindowCmd(CMP_TEST_UNIT, ENABLE);

    /* Enable CMP output */
    CMP_CompareOutCmd(CMP_TEST_UNIT, ENABLE);
    /* Enable VCOUT */
    CMP_PinVcoutCmd(CMP_TEST_UNIT, ENABLE);
}

/**
 * @brief  Configure TMR6 PWM output.
 * @param  None
 * @retval None
 */
static void Tmr6PwmConfig(void)
{
    stc_tmr6_init_t stcTmr6Init;
    stc_tmr6_pwm_init_t stcPwmInit;

    (void)TMR6_StructInit(&stcTmr6Init);
    (void)TMR6_PWM_StructInit(&stcPwmInit);

    /* Enable TMR6 peripheral clock */
    FCG_Fcg2PeriphClockCmd(TMR6_PERIP_CLK, ENABLE);

    /* Timer6 PWM output port configuration */
    GPIO_SetFunc(TMR6_1_PWMA_PORT, TMR6_1_PWMA_PIN, GPIO_FUNC_3);

    TMR6_Stop(TMR6_UNIT);
    /* Timer6 general count function configuration */
    stcTmr6Init.sw_count.u32ClockDiv = TMR6_CLK_DIV128;
    stcTmr6Init.u32PeriodValue = TMR6_PERIOD_VAL;
    (void)TMR6_Init(TMR6_UNIT, &stcTmr6Init);

    /* Configure PWM output */
    stcPwmInit.u32CompareValue = TMR6_CMP_VAL;
    stcPwmInit.u32CountDownMatchBPolarity = TMR6_PWM_HOLD;
    stcPwmInit.u32CountUpMatchBPolarity = TMR6_PWM_HOLD;
    stcPwmInit.u32CountDownMatchAPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32CountUpMatchAPolarity = TMR6_PWM_HIGH;
    stcPwmInit.u32UdfPolarity = TMR6_PWM_HOLD;
    stcPwmInit.u32OvfPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32StopPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32StartPolarity = TMR6_PWM_LOW;
    (void)TMR6_PWM_Init(TMR6_UNIT, TMR6_CH_A, &stcPwmInit);
    /* PWM pin function set */
    TMR6_SetFunc(TMR6_UNIT, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    /* PWM output command */
    TMR6_PWM_OutputCmd(TMR6_UNIT, TMR6_CH_A, ENABLE);

    /* Start timer6 */
    TMR6_Start(TMR6_UNIT);
}

/**
 * @brief  Main function of example project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();

    /* Configure TMR6 */
    Tmr6PwmConfig();

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
