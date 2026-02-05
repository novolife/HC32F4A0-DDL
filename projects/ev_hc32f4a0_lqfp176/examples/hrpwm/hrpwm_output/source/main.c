/**
 *******************************************************************************
 * @file  hrpwm/hrpwm_output/source/main.c
 * @brief This example demonstrates HRPWM output function with timer6 PWM.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2023-01-15       CDT             Modify structure stc_timer6_init_t to stc_tmr6_init_t
   2023-09-30       CDT             Modify for Peripheral clock command process
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
 * @addtogroup TIMER6_Hrpwm_Output
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU |\
                                         LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

#define TMR6_1_PWMA_PORT                (GPIO_PORT_B)
#define TMR6_1_PWMA_PIN                 (GPIO_PIN_09)
#define TMR6_1_PWMB_PORT                (GPIO_PORT_B)
#define TMR6_1_PWMB_PIN                 (GPIO_PIN_08)

#define TMR6_2_PWMA_PORT                (GPIO_PORT_B)
#define TMR6_2_PWMA_PIN                 (GPIO_PIN_07)
#define TMR6_2_PWMB_PORT                (GPIO_PORT_B)
#define TMR6_2_PWMB_PIN                 (GPIO_PIN_14)

#define TMR6_3_PWMA_PORT                (GPIO_PORT_A)
#define TMR6_3_PWMA_PIN                 (GPIO_PIN_10)
#define TMR6_3_PWMB_PORT                (GPIO_PORT_B)
#define TMR6_3_PWMB_PIN                 (GPIO_PIN_15)

#define TMR6_4_PWMA_PORT                (GPIO_PORT_E)
#define TMR6_4_PWMA_PIN                 (GPIO_PIN_14)
#define TMR6_4_PWMB_PORT                (GPIO_PORT_B)
#define TMR6_4_PWMB_PIN                 (GPIO_PIN_10)

#define TMR6_5_PWMA_PORT                (GPIO_PORT_C)
#define TMR6_5_PWMA_PIN                 (GPIO_PIN_10)
#define TMR6_5_PWMB_PORT                (GPIO_PORT_D)
#define TMR6_5_PWMB_PIN                 (GPIO_PIN_00)

#define TMR6_6_PWMA_PORT                (GPIO_PORT_A)
#define TMR6_6_PWMA_PIN                 (GPIO_PIN_02)
#define TMR6_6_PWMB_PORT                (GPIO_PORT_A)
#define TMR6_6_PWMB_PIN                 (GPIO_PIN_03)

#define TMR6_7_PWMA_PORT                (GPIO_PORT_A)
#define TMR6_7_PWMA_PIN                 (GPIO_PIN_04)
#define TMR6_7_PWMB_PORT                (GPIO_PORT_A)
#define TMR6_7_PWMB_PIN                 (GPIO_PIN_05)

#define TMR6_8_PWMA_PORT                (GPIO_PORT_C)
#define TMR6_8_PWMA_PIN                 (GPIO_PIN_02)
#define TMR6_8_PWMB_PORT                (GPIO_PORT_C)
#define TMR6_8_PWMB_PIN                 (GPIO_PIN_03)

/* Close the definition to disale the HRPWM function for the example. */
#define TEST_HRPWM_FUNC_ON              (1U)

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
 * @brief  Main function of TIMER6 compare output mode project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint32_t u32Period;
    uint32_t u32Compare;
    stc_tmr6_init_t stcTmr6Init;
    stc_tmr6_pwm_init_t stcPwmInit;
    stc_gpio_init_t stcGpioInit;
    stc_clock_freq_t stcClkFreq;
    uint8_t u8CalCode0, u8CalCode1;
    __UNUSED float32_t fPrecision0, fPrecision1;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_CLK_Init();
    BSP_IO_Init();
    BSP_LED_Init();

    (void)TMR6_StructInit(&stcTmr6Init);
    (void)TMR6_PWM_StructInit(&stcPwmInit);
    (void)GPIO_StructInit(&stcGpioInit);

#ifdef TEST_HRPWM_FUNC_ON
    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_HRPWM, ENABLE);

    (void)CLK_GetClockFreq(&stcClkFreq);

    /* Make sure that F(PCLK0) > 120M */
    if (DISABLE == HRPWM_CondConfirm()) {
        BSP_LED_On(LED_RED);
        for (;;) {
            ;
        }
    }

    if (LL_OK != HRPWM_CalibProcess(HRPWM_CALIB_UNIT0, &u8CalCode0)) {
        BSP_LED_On(LED_RED);
        for (;;) {
            ;
        }
    }

    if (LL_OK != HRPWM_CalibProcess(HRPWM_CALIB_UNIT1, &u8CalCode1)) {
        BSP_LED_On(LED_RED);
        for (;;) {
            ;
        }
    }

    /* Calculate calibrate precision for channel 1~channel 12, unit nanosecond */
    fPrecision0 = (float32_t)1000000000 / (float32_t)stcClkFreq.u32Pclk0Freq / (float32_t)u8CalCode0;
    /* Calculate calibrate precision for channel 13~channel 16, unit nanosecond */
    fPrecision1 = (float32_t)1000000000 / (float32_t)stcClkFreq.u32Pclk0Freq / (float32_t)u8CalCode1;

    /* HRPWM function configure for channel 1, PB9*/
    HRPWM_ChPositiveAdjustConfig(1U, 1U);
    HRPWM_ChPositiveAdjustCmd(1U, ENABLE);
    HRPWM_ChNegativeAdjustConfig(1U, 1U);
    HRPWM_ChNegativeAdjustCmd(1U, ENABLE);
    HRPWM_ChCmd(1U, ENABLE);

    /* HRPWM function configure for channel 5, PA10*/
    HRPWM_ChPositiveAdjustConfig(5U, u8CalCode0 / 2U);
    HRPWM_ChPositiveAdjustCmd(5U, ENABLE);
    HRPWM_ChNegativeAdjustConfig(5U, u8CalCode0 / 2U);
    HRPWM_ChNegativeAdjustCmd(5U, ENABLE);
    HRPWM_ChCmd(5U, ENABLE);

#endif
    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR6_1 | FCG2_PERIPH_TMR6_2 | \
                           FCG2_PERIPH_TMR6_3 | FCG2_PERIPH_TMR6_4 | \
                           FCG2_PERIPH_TMR6_5 | FCG2_PERIPH_TMR6_6 | \
                           FCG2_PERIPH_TMR6_7 | FCG2_PERIPH_TMR6_8, ENABLE);

    /* Timer6 PWM port configuration */
    GPIO_SetFunc(TMR6_1_PWMA_PORT, TMR6_1_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_1_PWMB_PORT, TMR6_1_PWMB_PIN, GPIO_FUNC_3);

    GPIO_SetFunc(TMR6_2_PWMA_PORT, TMR6_2_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_2_PWMB_PORT, TMR6_2_PWMB_PIN, GPIO_FUNC_3);

    GPIO_SetFunc(TMR6_3_PWMA_PORT, TMR6_3_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_3_PWMB_PORT, TMR6_3_PWMB_PIN, GPIO_FUNC_3);

    GPIO_SetFunc(TMR6_4_PWMA_PORT, TMR6_4_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_4_PWMB_PORT, TMR6_4_PWMB_PIN, GPIO_FUNC_3);

    GPIO_SetFunc(TMR6_5_PWMA_PORT, TMR6_5_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_5_PWMB_PORT, TMR6_5_PWMB_PIN, GPIO_FUNC_3);

    GPIO_SetFunc(TMR6_6_PWMA_PORT, TMR6_6_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_6_PWMB_PORT, TMR6_6_PWMB_PIN, GPIO_FUNC_3);

    GPIO_SetFunc(TMR6_7_PWMA_PORT, TMR6_7_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_7_PWMB_PORT, TMR6_7_PWMB_PIN, GPIO_FUNC_3);

    GPIO_SetFunc(TMR6_8_PWMA_PORT, TMR6_8_PWMA_PIN, GPIO_FUNC_3);
    GPIO_SetFunc(TMR6_8_PWMB_PORT, TMR6_8_PWMB_PIN, GPIO_FUNC_3);

    u32Period = 50U;
    /* Timer6 general count function configuration */
    stcTmr6Init.sw_count.u32CountMode = TMR6_MD_TRIANGLE;
    stcTmr6Init.sw_count.u32ClockDiv = TMR6_CLK_DIV1;
    stcTmr6Init.u32PeriodValue = u32Period;
    (void)TMR6_Init(CM_TMR6_1, &stcTmr6Init);
    (void)TMR6_Init(CM_TMR6_2, &stcTmr6Init);
    (void)TMR6_Init(CM_TMR6_3, &stcTmr6Init);
    (void)TMR6_Init(CM_TMR6_4, &stcTmr6Init);
    (void)TMR6_Init(CM_TMR6_5, &stcTmr6Init);
    (void)TMR6_Init(CM_TMR6_6, &stcTmr6Init);
    (void)TMR6_Init(CM_TMR6_7, &stcTmr6Init);
    (void)TMR6_Init(CM_TMR6_8, &stcTmr6Init);

    /* Compare register set */
    u32Compare = u32Period - 8UL;
    /* Configure PWM output */
    stcPwmInit.u32CompareValue = u32Compare;
    stcPwmInit.u32CountDownMatchBPolarity = TMR6_PWM_HOLD;
    stcPwmInit.u32CountUpMatchBPolarity = TMR6_PWM_HOLD;
    stcPwmInit.u32CountDownMatchAPolarity = TMR6_PWM_HOLD;
    stcPwmInit.u32CountUpMatchAPolarity = TMR6_PWM_INVT;
    stcPwmInit.u32UdfPolarity = TMR6_PWM_HOLD;
    stcPwmInit.u32OvfPolarity = TMR6_PWM_INVT;
    stcPwmInit.u32StopPolarity = TMR6_PWM_LOW;
    stcPwmInit.u32StartPolarity = TMR6_PWM_LOW;
    (void)TMR6_PWM_Init(CM_TMR6_1, TMR6_CH_A, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_2, TMR6_CH_A, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_3, TMR6_CH_A, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_4, TMR6_CH_A, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_5, TMR6_CH_A, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_6, TMR6_CH_A, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_7, TMR6_CH_A, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_8, TMR6_CH_A, &stcPwmInit);
    stcPwmInit.u32CountDownMatchBPolarity = TMR6_PWM_HOLD;
    stcPwmInit.u32CountUpMatchBPolarity = TMR6_PWM_INVT;
    stcPwmInit.u32CountDownMatchAPolarity = TMR6_PWM_HOLD;
    stcPwmInit.u32CountUpMatchAPolarity = TMR6_PWM_HOLD;
    (void)TMR6_PWM_Init(CM_TMR6_1, TMR6_CH_B, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_2, TMR6_CH_B, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_3, TMR6_CH_B, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_4, TMR6_CH_B, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_5, TMR6_CH_B, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_6, TMR6_CH_B, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_7, TMR6_CH_B, &stcPwmInit);
    (void)TMR6_PWM_Init(CM_TMR6_8, TMR6_CH_B, &stcPwmInit);
    /* PWM pin function set */
    TMR6_SetFunc(CM_TMR6_1, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_1, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_2, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_2, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_3, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_3, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_4, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_4, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_5, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_5, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_6, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_6, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_7, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_7, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_8, TMR6_CH_A, TMR6_PIN_CMP_OUTPUT);
    TMR6_SetFunc(CM_TMR6_8, TMR6_CH_B, TMR6_PIN_CMP_OUTPUT);
    /* PWM output command */
    TMR6_PWM_OutputCmd(CM_TMR6_1, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_1, TMR6_CH_B, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_2, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_2, TMR6_CH_B, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_3, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_3, TMR6_CH_B, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_4, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_4, TMR6_CH_B, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_5, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_5, TMR6_CH_B, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_6, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_6, TMR6_CH_B, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_7, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_7, TMR6_CH_B, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_8, TMR6_CH_A, ENABLE);
    TMR6_PWM_OutputCmd(CM_TMR6_8, TMR6_CH_B, ENABLE);

    /* Start timer6 */
    TMR6_SWSyncStart(TMR6_SW_SYNC_U1 | TMR6_SW_SYNC_U2 | TMR6_SW_SYNC_U3 \
                     | TMR6_SW_SYNC_U4 | TMR6_SW_SYNC_U5 | TMR6_SW_SYNC_U6 \
                     | TMR6_SW_SYNC_U7 | TMR6_SW_SYNC_U8);
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
