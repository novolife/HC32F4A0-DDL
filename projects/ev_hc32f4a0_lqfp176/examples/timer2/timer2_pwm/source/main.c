/**
 *******************************************************************************
 * @file  timer2/timer2_pwm/source/main.c
 * @brief Main program Timer2 PWM for the Device Driver Library.
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
 * @addtogroup TIMER2_PWM
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/**
 * Functions of this example.
 * Use one channel of Timer2 to output one PWM.
 */

/**
 * Timer2 unit and channel definitions for this example.
 * 'TMR2_UNIT' can be defined as CM_TMR2_<t>(t=1 ~ 4).
 * 'TMR2_CH' can be defined as TMR2_CH_x(x=A, B).
 */
#define TMR2_UNIT                       (CM_TMR2_1)
#define TMR2_CH                         (TMR2_CH_A)
#define TMR2_PERIP_CLK                  (FCG2_PERIPH_TMR2_1)

/* Timer2 PWM pin definitions. */
#define TMR2_PWM_PORT                   (GPIO_PORT_A)
#define TMR2_PWM_PIN                    (GPIO_PIN_02)
#define TMR2_PWM_PIN_FUNC               (GPIO_FUNC_16)

/* Timer2 clock source definitions. */
#define TMR2_CLK_SRC                    (TMR2_CLK_PCLK1)
#define TMR2_CLK_DIV                    (TMR2_CLK_DIV4)

/**
 * Calculate the compare value register according to clock source, the prescaler of the clock source and PWM frequency.
 * CompareValue = (Timer2ClockFrequency(Hz) / PwmFrequency(Hz) / 2) - 1.
 * In this example:
 *   Timer2ClockFrequency = MRC(8MHz) / Timer2ClockPrescaler(4) = 2000000Hz;
 *   PwmFrequency = 1000000Hz;
 *   CompareValue = (2000000 / 1000000 / 2) - 1 = 0.
 *
 * NOTE:
 *   Duty ratio is fixed as 50%.
 */
#define TMR2_CMP_VAL                    (0U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void Tmr2Config(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of timer2_pwm project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* Configures Timer2. */
    Tmr2Config();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);

    /* Starts Timer2 to start PWM output. */
    TMR2_Start(TMR2_UNIT, TMR2_CH);

    /***************** Configuration end, application start **************/

    for (;;) {
        /* Call TMR2_Stop(TMR2_UNIT, TMR2_CH) to stop the PWM output. */
    }
}

/**
 * @brief  Timer2 configuration.
 * @param  None
 * @retval None
 */
static void Tmr2Config(void)
{
    stc_tmr2_init_t stcTmr2Init;
    stc_tmr2_pwm_init_t stcPwmInit;

    /* 1. Enable Timer2 peripheral clock. */
    FCG_Fcg2PeriphClockCmd(TMR2_PERIP_CLK, ENABLE);

    /* 2. Set a default initialization value for stcTmr2Init. */
    (void)TMR2_StructInit(&stcTmr2Init);

    /* 3. Modifies the initialization values depends on the application. */
    stcTmr2Init.u32ClockSrc     = TMR2_CLK_SRC;
    stcTmr2Init.u32ClockDiv     = TMR2_CLK_DIV;
    stcTmr2Init.u32CompareValue = TMR2_CMP_VAL;
    (void)TMR2_Init(TMR2_UNIT, TMR2_CH, &stcTmr2Init);

    /* 4. PWM configuration. */
    GPIO_SetFunc(TMR2_PWM_PORT, TMR2_PWM_PIN, TMR2_PWM_PIN_FUNC);
    (void)TMR2_PWM_StructInit(&stcPwmInit);
    (void)TMR2_PWM_Init(TMR2_UNIT, TMR2_CH, &stcPwmInit);
    TMR2_PWM_OutputCmd(TMR2_UNIT, TMR2_CH, ENABLE);
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
