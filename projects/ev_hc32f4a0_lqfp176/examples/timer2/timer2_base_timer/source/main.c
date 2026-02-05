/**
 *******************************************************************************
 * @file  timer2/timer2_base_timer/source/main.c
 * @brief Main program Timer2 base timer for the Device Driver Library.
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
 * @addtogroup TIMER2_Base_Timer
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Timer2 unit and channel definitions for this example. */
#define TMR2_UNIT                       (CM_TMR2_1)
#define TMR2_CH                         (TMR2_CH_A)
#define TMR2_PERIPH_CLK                 (FCG2_PERIPH_TMR2_1)

/**
 * Use hardware trigger if needed. None-zero to enable.
 * Hardware trigger conditions control. The conditions that can start Timer2, \
 * stop Timer2 or clear counter of Timer2.
 */
#define TMR2_USE_HW_TRIG                (0U)

/**
 * Clock source for this example.
 * In this example:
 *   1. System clock is MRC@8MHz.
 *   2. PCLK1 is 8MHz.
 *   3. Use timer2 to count 10ms.
 *
 * A simple formula for calculating the compare value is:
 *   Tmr2CompareValue = (Tmr2Period(us) * [Tmr2ClockSource(MHz) / Tmr2ClockDiv]) - 1.
 */
#define TMR2_CLK_SRC                    (TMR2_CLK_PCLK1)
#define TMR2_CLK_DIV                    (TMR2_CLK_DIV4)
#define TMR2_CMP_VAL                    (20000UL - 1U)

/* Definitions about Timer2 interrupt for the example. */
#define TMR2_INT_TYPE                   (TMR2_INT_MATCH_CH_A)
#define TMR2_INT_PRIO                   (DDL_IRQ_PRIO_03)
#define TMR2_INT_SRC                    (INT_SRC_TMR2_1_CMP_A)
#define TMR2_INT_IRQn                   (INT050_IRQn)
#define TMR2_FLAG                       (TMR2_FLAG_MATCH_CH_A)

/**
 * Specify the hardware trigger conditions if enabled(TMR2_USE_HW_TRIG > 0U).
 * 'TMR2_START_COND' specifies the condition of starting Timer2.
 * 'TMR2_STOP_COND' specifies the condition of stopping Timer2.
 * NOTE:
 *   1. CANNOT specify a condition as both the start condition and the stop condition.
 *   2. Pin TIM2_<t>_PWMAx CANNOT be a trigger condition while it's edge is used as the synchronous clock.
 */
#if (TMR2_USE_HW_TRIG > 0U)
#define TMR2_START_COND                 (TMR2_START_COND_EVT)
/* BSP key K4 can generate the event 'EVT_SRC_PORT_EIRQ3' */
#define TMR2_TRIG_EVT                   (EVT_SRC_PORT_EIRQ3)

#define TMR2_STOP_COND                  (TMR2_STOP_COND_TRIG_FALLING)
#define TMR2_TRIG_PORT                  (GPIO_PORT_A)
#define TMR2_TRIG_PIN                   (GPIO_PIN_02)
#define TMR2_TRIG_PIN_FUNC              (GPIO_FUNC_16)
#endif /* #if (TMR2_USE_HW_TRIG > 0U) */

/* Indicate pin definition in this example. */
#define INDICATE_PORT                   (GPIO_PORT_A)
#define INDICATE_PIN                    (GPIO_PIN_10)
#define INDICATE_OUT_TOGGLE()           (GPIO_TogglePins(INDICATE_PORT, INDICATE_PIN))

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void IndicateConfig(void);

static void Tmr2Config(void);
static void Tmr2IrqConfig(void);
static void TMR2_Cmp_IrqCallback(void);

#if (TMR2_USE_HW_TRIG > 0U)
static void Tmr2TriggerCondConfig(void);
#endif

static void Tmr2Start(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of timer2_base_timer project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
#if (TMR2_USE_HW_TRIG > 0U)
    /* BSP key K4 can trigger Timer2 start. */
    BSP_KEY_Init();
#endif /* #if (TMR2_USE_HW_TRIG > 0U) */
    /* Configures indicator. */
    IndicateConfig();
    /* Configures Timer2. */
    Tmr2Config();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);

    /* Starts Timer2. */
    Tmr2Start();

    /***************** Configuration end, application start **************/

    for (;;) {
        /* See TMR2_Cmp_IrqCallback in this file. */
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

    /* 1. Enable Timer2 peripheral clock. */
    FCG_Fcg2PeriphClockCmd(TMR2_PERIPH_CLK, ENABLE);

    /* 2. Set a default initialization value for stcTmr2Init. */
    (void)TMR2_StructInit(&stcTmr2Init);

    /* 3. Modifies the initialization values depends on the application. */
    stcTmr2Init.u32ClockSrc     = TMR2_CLK_SRC;
    stcTmr2Init.u32ClockDiv     = TMR2_CLK_DIV;
    stcTmr2Init.u32Func         = TMR2_FUNC_CMP;
    stcTmr2Init.u32CompareValue = TMR2_CMP_VAL;
    (void)TMR2_Init(TMR2_UNIT, TMR2_CH, &stcTmr2Init);

    /* 4. Configures IRQ if needed. */
    Tmr2IrqConfig();

#if (TMR2_USE_HW_TRIG > 0U)
    /* 5. Configures hardware trigger condition if needed. */
    Tmr2TriggerCondConfig();
#endif /* #if (TMR2_USE_HW_TRIG > 0U) */
}

/**
 * @brief  Timer2 interrupt configuration.
 * @param  None
 * @retval None
 */
static void Tmr2IrqConfig(void)
{
    stc_irq_signin_config_t stcIrq;

    stcIrq.enIntSrc    = TMR2_INT_SRC;
    stcIrq.enIRQn      = TMR2_INT_IRQn;
    stcIrq.pfnCallback = &TMR2_Cmp_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);

    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, TMR2_INT_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);

    /* Enable the specified interrupts of Timer2. */
    TMR2_IntCmd(TMR2_UNIT, TMR2_INT_TYPE, ENABLE);
}

/**
 * @brief  Timer2 counter comparison match interrupt callback function.
 * @param  None
 * @retval None
 */
static void TMR2_Cmp_IrqCallback(void)
{
    TMR2_ClearStatus(TMR2_UNIT, TMR2_FLAG);
    INDICATE_OUT_TOGGLE();
}

#if (TMR2_USE_HW_TRIG > 0U)
/**
 * @brief  Configure the conditions which are used to start and stop Timer2.
 * @param  None
 * @retval None
 */
static void Tmr2TriggerCondConfig(void)
{
    /* TMR2 start condition. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    AOS_SetTriggerEventSrc(AOS_TMR2, TMR2_TRIG_EVT);
    TMR2_HWStartCondCmd(TMR2_UNIT, TMR2_CH, TMR2_START_COND, ENABLE);

    /* TMR2 stop condition. */
    GPIO_SetFunc(TMR2_TRIG_PORT, TMR2_TRIG_PIN, TMR2_TRIG_PIN_FUNC);
    /* Configures the filter of pin TRIG if needed. */
    TMR2_SetFilterClockDiv(TMR2_UNIT, TMR2_CH, TMR2_FILTER_CLK_DIV64);
    TMR2_FilterCmd(TMR2_UNIT, TMR2_CH, ENABLE);
    TMR2_HWStopCondCmd(TMR2_UNIT, TMR2_CH, TMR2_STOP_COND, ENABLE);
}
#endif

/**
 * @brief  Start Timer2.
 * @param  None
 * @retval None
 */
static void Tmr2Start(void)
{
    /**
     * If a peripheral is used to generate the event which is used as a hardware trigger condition of Timer2, \
     *   call the API of the peripheral to start the peripheral here or anywhere else you need.
     * The following operations are only used in this example.
     */

#if ((TMR2_USE_HW_TRIG == 0U) || \
     ((TMR2_USE_HW_TRIG > 0U) && (TMR2_START_COND == TMR2_START_COND_INVD)))
    TMR2_Start(TMR2_UNIT, TMR2_CH);
#else
    /* Press BSP key K4 to start Timer2. */
#endif
}

/**
 * @brief  Indicator configuration.
 * @param  None
 * @retval None
 */
static void IndicateConfig(void)
{
    stc_gpio_init_t stcGpio;

    (void)GPIO_StructInit(&stcGpio);
    (void)GPIO_Init(INDICATE_PORT, INDICATE_PIN, &stcGpio);
    /* Output enable */
    GPIO_OutputCmd(INDICATE_PORT, INDICATE_PIN, ENABLE);
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
