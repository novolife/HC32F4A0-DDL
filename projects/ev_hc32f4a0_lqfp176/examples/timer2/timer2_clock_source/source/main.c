/**
 *******************************************************************************
 * @file  timer2/timer2_clock_source/source/main.c
 * @brief Main program Timer2 clock source for the Device Driver Library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2023-09-30       CDT             Refine code due to some of member type of struct stc_tmra_init_t is changed
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
 * @addtogroup TIMER2_Clock_Source
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * Timer2 unit and channel definitions for this example.
 * 'TMR2_UNIT' can be defined as CM_TMR2_<t>(t=1 ~ 4).
 * 'TMR2_CH' can de defined as TMR2_CH_x(x=A, B).
 */
#define TMR2_UNIT                       (CM_TMR2_1)
#define TMR2_CH                         (TMR2_CH_A)
#define TMR2_PERIPH_CLK                 (FCG2_PERIPH_TMR2_1)
#define TMR2_FLAG                       (TMR2_FLAG_MATCH_CH_A)

/* Specifies clock source for TMR2. @ref TMR2_Clock_Source */
#define TMR2_CLK_SRC                    (TMR2_CLK_PIN_CLK)

/* Different clock source, different configuration. */
#if (TMR2_CLK_SRC == TMR2_CLK_PCLK1)
/* Clock divider and compare value. 50ms */
#define TMR2_CLK_DIV                    (TMR2_CLK_DIV128)
#define TMR2_CMP_VAL                    (46875U - 1U)

#elif (TMR2_CLK_SRC == TMR2_CLK_TRIG_RISING) || (TMR2_CLK_SRC == TMR2_CLK_TRIG_FALLING)
#define TMR2_TRIG_PORT                  (GPIO_PORT_A)
#define TMR2_TRIG_PIN                   (GPIO_PIN_02)
#define TMR2_TRIG_PIN_FUNC              (GPIO_FUNC_16)

/* Rising or falling of pin TRIGA/B is from PWM(1MHz, 62.5% high duty) of TimerA. */
#define TMRA_UNIT                       (CM_TMRA_1)
#define TMRA_CH                         (TMRA_CH2)
#define TMRA_PERIPH_CLK                 (FCG2_PERIPH_TMRA_1)
#define TMRA_PWM_PORT                   (GPIO_PORT_E)
#define TMRA_PWM_PIN                    (GPIO_PIN_11)
#define TMRA_PWM_PIN_FUNC               (GPIO_FUNC_4)

/* Compare value. Count 25000 rising or falling. */
#define TMR2_CMP_VAL                    (25000U - 1U)

#elif (TMR2_CLK_SRC == TMR2_CLK_EVT)
/* Key K10(PA0) generates. */
#define TMR2_CNT_EVT                    (EVT_SRC_PORT_EIRQ0)
/* Compare value. Count 5 times. */
#define TMR2_CMP_VAL                    (5U - 1U)

#elif (TMR2_CLK_SRC == TMR2_CLK_TMR6_OVF)
#define TMR6_UNIT                       (CM_TMR6_1)
#define TMR6_PERIPH_CLK                 (FCG2_PERIPH_TMR6_1)
#define TMR6_CNT_DIR                    (TMR6_CNT_UP)
/* Compare value. Count 500 times, total 100ms. */
#define TMR2_CMP_VAL                    (100U - 1U)

#elif (TMR2_CLK_SRC == TMR2_CLK_TMR6_UDF)
#define TMR6_UNIT                       (CM_TMR6_1)
#define TMR6_PERIPH_CLK                 (FCG2_PERIPH_TMR6_1)
#define TMR6_CNT_DIR                    (TMR6_CNT_DOWN)
/* Compare value. Count 500 times, total 100ms. */
#define TMR2_CMP_VAL                    (100U - 1U)

#elif (TMR2_CLK_SRC == TMR2_CLK_LRC) || (TMR2_CLK_SRC == TMR2_CLK_XTAL32)
/* Clock divider and compare value. 250ms */
#define TMR2_CLK_DIV                    (TMR2_CLK_DIV1)
#define TMR2_CMP_VAL                    (8192U - 1U)

#elif (TMR2_CLK_SRC == TMR2_CLK_PIN_CLK)
#define TMR2_CLK_PORT                   (GPIO_PORT_E)
#define TMR2_CLK_PIN                    (GPIO_PIN_03)
#define TMR2_CLK_PIN_FUNC               (GPIO_FUNC_16)
/* Use the 1MHz PWM output from timerA as the clock source of timer2 */
#define TMRA_UNIT                       (CM_TMRA_1)
#define TMRA_CH                         (TMRA_CH2)
#define TMRA_PERIPH_CLK                 (FCG2_PERIPH_TMRA_1)
#define TMRA_PWM_PORT                   (GPIO_PORT_E)
#define TMRA_PWM_PIN                    (GPIO_PIN_11)
#define TMRA_PWM_PIN_FUNC               (GPIO_FUNC_4)

/* Clock divider and compare value. 500ms */
#define TMR2_CLK_DIV                    (TMR2_CLK_DIV32)
#define TMR2_CMP_VAL                    (15625U - 1U)
#endif

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void Tmr2Config(void);
static void Tmr2ClockSourceConfig(void);
static void Tmr2ClockSourceStart(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of timer2_clock_source project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);
    /* System clock is configured as 240MHz. */
    BSP_CLK_Init();
    /* BSP IO */
    BSP_IO_Init();
    /* BSP led */
    BSP_LED_Init();
    /* Configures Timer2. */
    Tmr2Config();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);
    /* Start Timer2. */
    TMR2_Start(TMR2_UNIT, TMR2_CH);

    /***************** Configuration end, application start **************/

    for (;;) {
        if (TMR2_GetStatus(TMR2_UNIT, TMR2_FLAG) == SET) {
            /* Counter matchs the specified compare value. */
            TMR2_ClearStatus(TMR2_UNIT, TMR2_FLAG);

#if (TMR2_CLK_SRC == TMR2_CLK_LRC) || (TMR2_CLK_SRC == TMR2_CLK_PIN_CLK) || \
    (TMR2_CLK_SRC == TMR2_CLK_XTAL32)
            /* Delay at least 6 cycles of asynchronous clock. */
            DDL_DelayMS(1U);
#endif
            BSP_LED_Toggle(LED_BLUE);
        }
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

    /* 1. Configures clock source of timer2. */
    Tmr2ClockSourceConfig();
    Tmr2ClockSourceStart();

    /* 2. Enable Timer2 peripheral clock. */
    FCG_Fcg2PeriphClockCmd(TMR2_PERIPH_CLK, ENABLE);

    /* 3. Set a default initialization value for stcTmr2Init. */
    (void)TMR2_StructInit(&stcTmr2Init);

    /* 4. Modifies the initialization values depends on the application. */
    stcTmr2Init.u32ClockSrc = TMR2_CLK_SRC;
#if (TMR2_CLK_SRC == TMR2_CLK_PCLK1)   || \
    (TMR2_CLK_SRC == TMR2_CLK_LRC)     || \
    (TMR2_CLK_SRC == TMR2_CLK_XTAL32)  || \
    (TMR2_CLK_SRC == TMR2_CLK_PIN_CLK)
    stcTmr2Init.u32ClockDiv = TMR2_CLK_DIV;
#endif
    stcTmr2Init.u32CompareValue = TMR2_CMP_VAL;
    (void)TMR2_Init(TMR2_UNIT, TMR2_CH, &stcTmr2Init);
}

/**
 * @brief  Timer2 clock source configuration.
 * @param  None
 * @retval None
 */
static void Tmr2ClockSourceConfig(void)
{
#if (TMR2_CLK_SRC == TMR2_CLK_PCLK1)
    /* Not needed here. */
#elif (TMR2_CLK_SRC == TMR2_CLK_TRIG_RISING) || (TMR2_CLK_SRC == TMR2_CLK_TRIG_FALLING)
    GPIO_SetFunc(TMR2_TRIG_PORT, TMR2_TRIG_PIN, TMR2_TRIG_PIN_FUNC);
    /* Rising or falling of pin TRIGA/B is from PWM(1MHz, 62.5% high duty) of TimerA. */
    stc_tmra_init_t stcTmraInit;
    stc_tmra_pwm_init_t stcPwmInit;

    FCG_Fcg2PeriphClockCmd(TMRA_PERIPH_CLK, ENABLE);

    (void)TMRA_StructInit(&stcTmraInit);
    stcTmraInit.sw_count.u8ClockDiv = TMRA_CLK_DIV1;
    stcTmraInit.u32PeriodValue = 240U - 1U;
    (void)TMRA_Init(TMRA_UNIT, &stcTmraInit);

    TMRA_SetFunc(TMRA_UNIT, TMRA_CH, TMRA_FUNC_CMP);

    (void)TMRA_PWM_StructInit(&stcPwmInit);
    stcPwmInit.u32CompareValue = 150U - 1U;
    (void)TMRA_PWM_Init(TMRA_UNIT, TMRA_CH, &stcPwmInit);
    TMRA_PWM_OutputCmd(TMRA_UNIT, TMRA_CH, ENABLE);
    GPIO_SetFunc(TMRA_PWM_PORT, TMRA_PWM_PIN, TMRA_PWM_PIN_FUNC);

#elif (TMR2_CLK_SRC == TMR2_CLK_EVT)
    /* Press BSP key K10 to generate the event for timer2 counting. */
    BSP_KEY_Init();
    /* Enable AOS function. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    /* Set the event for Timer2 capturing. */
    AOS_SetTriggerEventSrc(AOS_TMR2, TMR2_CNT_EVT);
#elif (TMR2_CLK_SRC == TMR2_CLK_TMR6_OVF) || (TMR2_CLK_SRC == TMR2_CLK_TMR6_UDF)
    stc_tmr6_init_t stcTmr6Init;
    (void)TMR6_StructInit(&stcTmr6Init);
    stcTmr6Init.sw_count.u32CountDir = TMR6_CNT_DIR;
    stcTmr6Init.sw_count.u32ClockDiv = TMR6_CLK_DIV16;
    stcTmr6Init.u32PeriodValue = 7500UL;
    FCG_Fcg2PeriphClockCmd(TMR6_PERIPH_CLK, ENABLE);
    (void)TMR6_Init(TMR6_UNIT, &stcTmr6Init);

    /* NOTE: PCLK0(for TMR6) and PCLK1(for TMR2) must be the same frequency. */
    CLK_SetClockDiv(CLK_BUS_PCLK0,  CLK_PCLK0_DIV2);

#elif (TMR2_CLK_SRC == TMR2_CLK_LRC)
    (void)CLK_LrcCmd(ENABLE);
#elif (TMR2_CLK_SRC == TMR2_CLK_XTAL32)
    (void)CLK_Xtal32Cmd(ENABLE);
#elif (TMR2_CLK_SRC == TMR2_CLK_PIN_CLK)
    /* Input clock via the specified CLK pin for timer2. */
    /* Use TimerA to output PWM with a frequency of 1MHz. */
    stc_tmra_init_t stcTmraInit;
    stc_tmra_pwm_init_t stcPwmInit;

    FCG_Fcg2PeriphClockCmd(TMRA_PERIPH_CLK, ENABLE);

    (void)TMRA_StructInit(&stcTmraInit);
    stcTmraInit.sw_count.u8ClockDiv = TMRA_CLK_DIV1;
    stcTmraInit.u32PeriodValue = 240U - 1U;
    (void)TMRA_Init(TMRA_UNIT, &stcTmraInit);

    TMRA_SetFunc(TMRA_UNIT, TMRA_CH, TMRA_FUNC_CMP);

    (void)TMRA_PWM_StructInit(&stcPwmInit);
    stcPwmInit.u32CompareValue = 60U - 1U;
    (void)TMRA_PWM_Init(TMRA_UNIT, TMRA_CH, &stcPwmInit);
    TMRA_PWM_OutputCmd(TMRA_UNIT, TMRA_CH, ENABLE);
    GPIO_SetFunc(TMRA_PWM_PORT, TMRA_PWM_PIN, TMRA_PWM_PIN_FUNC);

    /* !!! Pin TIM2_<t>_CLKA/B set function. */
    GPIO_SetFunc(TMR2_CLK_PORT, TMR2_CLK_PIN, TMR2_CLK_PIN_FUNC);
#endif
}

/**
 * @brief  Start clock source of timer2.
 * @param  None
 * @retval None
 */
static void Tmr2ClockSourceStart(void)
{
#if (TMR2_CLK_SRC == TMR2_CLK_PCLK1)
    /* Not needed here. */
#elif (TMR2_CLK_SRC == TMR2_CLK_TRIG_RISING)
    TMRA_Start(TMRA_UNIT);
    /* Make rising edges on the specified TRIG pin. */
#elif (TMR2_CLK_SRC == TMR2_CLK_TRIG_FALLING)
    /* Make falling edges on the specified TRIG pin. */
#elif (TMR2_CLK_SRC == TMR2_CLK_EVT)
    /* Press BSP key K10 to generate the event for timer2 counting. */
#elif (TMR2_CLK_SRC == TMR2_CLK_TMR6_OVF) || (TMR2_CLK_SRC == TMR2_CLK_TMR6_UDF)
    TMR6_Start(TMR6_UNIT);
#elif (TMR2_CLK_SRC == TMR2_CLK_LRC)
    /* Not needed here. */
#elif (TMR2_CLK_SRC == TMR2_CLK_XTAL32)
    /* Not needed here. */
#elif (TMR2_CLK_SRC == TMR2_CLK_PIN_CLK)
    /* Input clock via the specified CLK pin for timer2. */
    TMRA_Start(TMRA_UNIT);
#endif
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
