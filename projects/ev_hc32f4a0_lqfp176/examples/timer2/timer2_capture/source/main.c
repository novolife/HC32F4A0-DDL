/**
 *******************************************************************************
 * @file  timer2/timer2_capture/source/main.c
 * @brief Main program Timer2 capture for the Device Driver Library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2023-01-15       CDT             Macro definition fixed
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
 * @addtogroup TIMER2_Capture
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
 * 'FUNC_CAPTURE_EVENT': Measure the time that between two occurrences of a same event.
 * 'FUNC_MEASURE_PULSE_WIDTH': Measure the pulse width of the square wave input from pin TIM2_<t>_PWMx(t=1 ~ 4, x=A, B).
 * 'FUNC_MEASURE_PERIOD': Measure the period of the square wave input from pin TIM2_<t>_PWMx.
 */
#define FUNC_CAPTURE_EVENT              (0U)
#define FUNC_MEASURE_PULSE_WIDTH        (1U)
#define FUNC_MEASURE_PERIOD             (2U)

/* Select the function of this example. */
#define EXAMPLE_FUNC                    (FUNC_CAPTURE_EVENT)

/**
 * Timer2 unit and channel definitions for this example.
 * 'TMR2_UNIT' can be defined as CM_TMR2_<t>(t=1 ~ 4).
 * 'TMR2_CH' can de defined as TMR2_CH_x(x=A, B).
 */
#define TMR2_UNIT                       (CM_TMR2_1)
#define TMR2_CH                         (TMR2_CH_A)
#define TMR2_PERIPH_CLK                 (FCG2_PERIPH_TMR2_1)

/**
 * Define configuration values according to the function of this example just selected.
 * In this example:
 *     System clock is 240MHz
 *     Set PCLK1(120MHz) as the clock source of Timer2.
 *     Timer2 clock frequency is 120/1 = 120MHz, clock cycle is 1/120 (us).
 *
 *    The maximum frequency input from pin TIM2_<t>_PWMx is PCLK1/3(typical value). \
 *        1MHz will be used in this example.
 */
#define TMR2_CLK_SRC                    (TMR2_CLK_PCLK1)
#define TMR2_CLK_DIV                    (TMR2_CLK_DIV1)
#define TMR2_CLK_FREQ                   (120000000UL)

#if (EXAMPLE_FUNC == FUNC_CAPTURE_EVENT)
#define TMR2_START_COND                 (TMR2_START_COND_EVT)
#define TMR2_STOP_COND                  (TMR2_STOP_COND_INVD)
#define TMR2_CLR_COND                   (TMR2_CLR_COND_INVD)
#define TMR2_CAPT_COND                  (TMR2_CAPT_COND_EVT)
#define TMR2_CAPT_EVT                   (EVT_SRC_PORT_EIRQ3)
#define TMR2_CAPT_CNT                   (2U)

#elif (EXAMPLE_FUNC == FUNC_MEASURE_PULSE_WIDTH)
#define TMR2_START_COND                 (TMR2_START_COND_TRIG_RISING)
#define TMR2_STOP_COND                  (TMR2_STOP_COND_TRIG_FALLING)
#define TMR2_CLR_COND                   (TMR2_CLR_COND_TRIG_FALLING)
#define TMR2_CAPT_COND                  (TMR2_CAPT_COND_TRIG_FALLING)
#define TMR2_CAPT_PORT                  (GPIO_PORT_A)
#define TMR2_CAPT_PIN                   (GPIO_PIN_02)
#define TMR2_CAPT_PIN_FUNC              (GPIO_FUNC_16)
#define TMR2_CAPT_CNT                   (11U)

#elif (EXAMPLE_FUNC == FUNC_MEASURE_PERIOD)
#define TMR2_START_COND                 (TMR2_START_COND_TRIG_RISING)
#define TMR2_STOP_COND                  (TMR2_STOP_COND_INVD)
#define TMR2_CLR_COND                   (TMR2_CLR_COND_TRIG_RISING)
#define TMR2_CAPT_COND                  (TMR2_CAPT_COND_TRIG_RISING)
#define TMR2_CAPT_PORT                  (GPIO_PORT_A)
#define TMR2_CAPT_PIN                   (GPIO_PIN_02)
#define TMR2_CAPT_PIN_FUNC              (GPIO_FUNC_16)
#define TMR2_CAPT_CNT                   (21U)
#else
#error "Function is NOT supported!!!"
#endif

#if (EXAMPLE_FUNC == FUNC_MEASURE_PULSE_WIDTH) || \
    (EXAMPLE_FUNC == FUNC_MEASURE_PERIOD)
/* Use the 1MHz PWM output from timerA as the capture source of timer2 */
#define TMRA_UNIT                       (CM_TMRA_1)
#define TMRA_CH                         (TMRA_CH2)
#define TMRA_PERIPH_CLK                 (FCG2_PERIPH_TMRA_1)
#define TMRA_PWM_PORT                   (GPIO_PORT_E)
#define TMRA_PWM_PIN                    (GPIO_PIN_11)
#define TMRA_PWM_PIN_FUNC               (GPIO_FUNC_4)
#endif

/**
 * Definitions about Timer2 interrupt for the example.
 * Timer2 independent IRQn: [INT000_IRQn, INT031_IRQn], [INT050_IRQn, INT055_IRQn].
 */
#define TMR2_INT_CMP_TYPE               (TMR2_INT_MATCH_CH_A)
#define TMR2_INT_CMP_PRIO               (DDL_IRQ_PRIO_05)
#define TMR2_INT_CMP_SRC                (INT_SRC_TMR2_1_CMP_A)
#define TMR2_INT_CMP_IRQn               (INT053_IRQn)
#define TRM2_CMP_FLAG                   (TMR2_FLAG_MATCH_CH_A)

#define TMR2_INT_OVF_TYPE               (TMR2_INT_OVF_CH_A)
#define TMR2_INT_OVF_PRIO               (DDL_IRQ_PRIO_04)
#define TMR2_INT_OVF_SRC                (INT_SRC_TMR2_1_OVF_A)
#define TMR2_INT_OVF_IRQn               (INT054_IRQn)
#define TMR2_OVF_FLAG                   (TMR2_INT_OVF_CH_A)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void Tmr2Config(void);
static void Tmr2IrqConfig(void);
static void Tmr2CaptureCondConfig(void);
static void Tmr2CaptureCondStart(void);

static void TMR2_Ovf_IrqCallback(void);
static void TMR2_Cmp_IrqCallback(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
__IO static uint32_t m_u32CaptureCount = 0U;
__IO static uint32_t m_u32OvfCount     = 0U;
static uint32_t m_au32CaptureTime[TMR2_CAPT_CNT];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of timer2_capture project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint32_t i;
    uint32_t u32Temp;
    float32_t f32Temp;
#if (EXAMPLE_FUNC == FUNC_CAPTURE_EVENT)
    uint8_t u8FirstCapture = 0U;
#endif
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);
    /* System clock is configured as 240MHz. */
    BSP_CLK_Init();
    /* Initializes UART for debug printing. Baudrate is 115200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configures Timer2. */
    Tmr2Config();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* Start the peripheral to generate the condition for Timer2 capturing. */
    Tmr2CaptureCondStart();

    /***************** Configuration end, application start **************/

    for (;;) {
#if (EXAMPLE_FUNC == FUNC_CAPTURE_EVENT)
        if (m_u32CaptureCount == 1U) {
            if (u8FirstCapture == 0U) {
                DDL_Printf("First capturing.\r\n");
                u8FirstCapture = 1U;
            }
        }
#endif
        if (m_u32CaptureCount >= TMR2_CAPT_CNT) {
#if (EXAMPLE_FUNC == FUNC_CAPTURE_EVENT)
            (void)i;
            u8FirstCapture = 0U;
            TMR2_Stop(TMR2_UNIT, TMR2_CH);

            if (m_u32OvfCount > 0U) {
                u32Temp = m_u32OvfCount * 65536UL + m_au32CaptureTime[1U] - m_au32CaptureTime[0U];
            } else {
                if (m_au32CaptureTime[0U] == 0U) {
                    u32Temp = m_au32CaptureTime[1U] + 2U;
                } else {
                    if (m_au32CaptureTime[1U] > m_au32CaptureTime[0U]) {
                        u32Temp = m_au32CaptureTime[1U] - m_au32CaptureTime[0U];
                    } else {
                        DDL_Printf("Capture error!!!! m_u32OvfCount should bigger than 0.\r\n");
                        for (;;) {
                            /* stop. to be handle */
                        }
                    }
                }
            }
            f32Temp = (float32_t)(u32Temp) / ((float32_t)TMR2_CLK_FREQ);
            DDL_Printf("Capture event completed: %u overflow, %u timer2 cycles, %uuS.\r\n", \
                       (unsigned int)m_u32OvfCount, (unsigned int)u32Temp, (unsigned int)(f32Temp * 1000000UL));
            TMR2_SetCountValue(TMR2_UNIT, TMR2_CH, 0U);
            m_u32OvfCount     = 0U;
            m_u32CaptureCount = 0U;

#elif (EXAMPLE_FUNC == FUNC_MEASURE_PULSE_WIDTH)
            u32Temp = 0U;
            /* The first capturing value is invalid. */
            for (i = 1UL; i < TMR2_CAPT_CNT; i++) {
                u32Temp += (m_au32CaptureTime[i] + 2U);
            }
            f32Temp = (float32_t)u32Temp / ((float32_t)(TMR2_CAPT_CNT - 1U));
            DDL_Printf("Calculate pulse width completed: %u timer2 cycles, %unS.\r\n", \
                       (unsigned int)f32Temp, (unsigned int)((f32Temp * 1000000000UL) / (float32_t)TMR2_CLK_FREQ));
            m_u32CaptureCount = 0U;
            DDL_DelayMS(500U);

#elif (EXAMPLE_FUNC == FUNC_MEASURE_PERIOD)
            u32Temp = 0U;
            /* The first capturing value is invalid. */
            for (i = 1UL; i < TMR2_CAPT_CNT; i++) {
                u32Temp += (m_au32CaptureTime[i] + 2U);
            }
            f32Temp = (float32_t)u32Temp / ((float32_t)(TMR2_CAPT_CNT - 1U));
            DDL_Printf("Calculate period completed: %u timer2 cycles, %unS.\r\n", \
                       (unsigned int)f32Temp, (unsigned int)((f32Temp * 1000000000UL) / (float32_t)TMR2_CLK_FREQ));
            m_u32CaptureCount = 0U;
            DDL_DelayMS(500U);

#endif

#if (TMR2_START_COND != TMR2_START_COND_INVD)
            TMR2_HWStartCondCmd(TMR2_UNIT, TMR2_CH, TMR2_START_COND, ENABLE);
#endif
#if (TMR2_CLR_COND != TMR2_CLR_COND_INVD)
            TMR2_HWClearCondCmd(TMR2_UNIT, TMR2_CH, TMR2_CLR_COND, ENABLE);
#endif
#if (TMR2_CAPT_COND != TMR2_CAPT_COND_INVD)
            TMR2_HWCaptureCondCmd(TMR2_UNIT, TMR2_CH, TMR2_CAPT_COND, ENABLE);
#endif
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

    /* 1. Enable Timer2 peripheral clock. */
    FCG_Fcg2PeriphClockCmd(TMR2_PERIPH_CLK, ENABLE);

    /* 2. Set a default initialization value for stcTmr2Init. */
    (void)TMR2_StructInit(&stcTmr2Init);

    /* 3. Modifies the initialization values depends on the application. */
    stcTmr2Init.u32Func     = TMR2_FUNC_CAPT;
    stcTmr2Init.u32ClockSrc = TMR2_CLK_SRC;
    stcTmr2Init.u32ClockDiv = TMR2_CLK_DIV;
    (void)TMR2_Init(TMR2_UNIT, TMR2_CH, &stcTmr2Init);

    /* 4. Configures IRQ. */
    Tmr2IrqConfig();

    /* 5. Configures capture condition. */
    Tmr2CaptureCondConfig();
}

/**
 * @brief  Timer2 interrupt configuration.
 * @param  None
 * @retval None
 */
static void Tmr2IrqConfig(void)
{
    stc_irq_signin_config_t stcIrq;

    stcIrq.enIntSrc    = TMR2_INT_CMP_SRC;
    stcIrq.enIRQn      = TMR2_INT_CMP_IRQn;
    stcIrq.pfnCallback = &TMR2_Cmp_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);

    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, TMR2_INT_CMP_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);

    /* Overflow interrupt. */
    stcIrq.enIntSrc    = TMR2_INT_OVF_SRC;
    stcIrq.enIRQn      = TMR2_INT_OVF_IRQn;
    stcIrq.pfnCallback = &TMR2_Ovf_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);

    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
    NVIC_SetPriority(stcIrq.enIRQn, TMR2_INT_OVF_PRIO);
    NVIC_EnableIRQ(stcIrq.enIRQn);

    /* Enable the specified interrupts of Timer2. */
    TMR2_IntCmd(TMR2_UNIT, TMR2_INT_CMP_TYPE | TMR2_INT_OVF_TYPE, ENABLE);
}

/**
 * @brief  Specifies the hardware trigger conditions for capturing and \
 *         configures the peripheral which will generate the condition that to be captured by Timer2.
 * @param  None
 * @retval None
 */
static void Tmr2CaptureCondConfig(void)
{
#if (EXAMPLE_FUNC == FUNC_CAPTURE_EVENT)
    /* Press BSP key K4 to generate the event. */
    BSP_KEY_Init();
    /* Enable AOS function. */
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);
    /* Set the event for Timer2 capturing. */
    AOS_SetTriggerEventSrc(AOS_TMR2, TMR2_CAPT_EVT);

#elif ((EXAMPLE_FUNC == FUNC_MEASURE_PULSE_WIDTH) || (EXAMPLE_FUNC == FUNC_MEASURE_PERIOD))
    /* Use TimerA to output PWM with a frequency of 1MHz and a duty cycle of 58%. */
    stc_tmra_init_t stcTmraInit;
    stc_tmra_pwm_init_t stcPwmInit;

    FCG_Fcg2PeriphClockCmd(TMRA_PERIPH_CLK, ENABLE);

    /* TimerA unit 1: clock source PCLK0(240MHZ). Divider: 1. Final count clock frequency 240MHz. */
    (void)TMRA_StructInit(&stcTmraInit);
    stcTmraInit.sw_count.u8ClockDiv = TMRA_CLK_DIV1;
    stcTmraInit.u32PeriodValue = 240U - 1U;
    (void)TMRA_Init(TMRA_UNIT, &stcTmraInit);

    TMRA_SetFunc(TMRA_UNIT, TMRA_CH, TMRA_FUNC_CMP);

    (void)TMRA_PWM_StructInit(&stcPwmInit);
    stcPwmInit.u32CompareValue  = 100U;
    (void)TMRA_PWM_Init(TMRA_UNIT, TMRA_CH, &stcPwmInit);
    TMRA_PWM_OutputCmd(TMRA_UNIT, TMRA_CH, ENABLE);

    GPIO_SetFunc(TMRA_PWM_PORT, TMRA_PWM_PIN, TMRA_PWM_PIN_FUNC);

    /* Configure the wave input pin. */
    GPIO_SetFunc(TMR2_CAPT_PORT, TMR2_CAPT_PIN, TMR2_CAPT_PIN_FUNC);
#endif

#if (TMR2_START_COND != TMR2_START_COND_INVD)
    TMR2_HWStartCondCmd(TMR2_UNIT, TMR2_CH, TMR2_START_COND, ENABLE);
#endif
#if (TMR2_STOP_COND != TMR2_STOP_COND_INVD)
    TMR2_HWStopCondCmd(TMR2_UNIT, TMR2_CH, TMR2_STOP_COND, ENABLE);
#endif
#if (TMR2_CLR_COND != TMR2_CLR_COND_INVD)
    TMR2_HWClearCondCmd(TMR2_UNIT, TMR2_CH, TMR2_CLR_COND, ENABLE);
#endif
#if (TMR2_CAPT_COND != TMR2_CAPT_COND_INVD)
    TMR2_HWCaptureCondCmd(TMR2_UNIT, TMR2_CH, TMR2_CAPT_COND, ENABLE);
#endif
}

/**
 * @brief  Timer2 counter comparison match interrupt callback function.
 * @param  None
 * @retval None
 */
static void TMR2_Cmp_IrqCallback(void)
{
    TMR2_ClearStatus(TMR2_UNIT, TRM2_CMP_FLAG);
    if (m_u32CaptureCount < TMR2_CAPT_CNT) {
        m_au32CaptureTime[m_u32CaptureCount] = TMR2_GetCompareValue(TMR2_UNIT, TMR2_CH);
        m_u32CaptureCount++;
        if (m_u32CaptureCount >= TMR2_CAPT_CNT) {
            /* Disable the conditions for calculating the result, if needed. */
#if (TMR2_START_COND != TMR2_START_COND_INVD)
            TMR2_HWStartCondCmd(TMR2_UNIT, TMR2_CH, TMR2_START_COND, DISABLE);
#endif
#if (TMR2_CLR_COND != TMR2_CLR_COND_INVD)
            TMR2_HWClearCondCmd(TMR2_UNIT, TMR2_CH, TMR2_CLR_COND, DISABLE);
#endif
#if (TMR2_CAPT_COND != TMR2_CAPT_COND_INVD)
            TMR2_HWCaptureCondCmd(TMR2_UNIT, TMR2_CH, TMR2_CAPT_COND, DISABLE);
#endif
        }
    }
}

/**
 * @brief  Timer2 counter overflow interrupt callback function.
 * @param  None
 * @retval None
 */
static void TMR2_Ovf_IrqCallback(void)
{
    TMR2_ClearStatus(TMR2_UNIT, TMR2_OVF_FLAG);
    if (m_u32CaptureCount < TMR2_CAPT_CNT) {
        m_u32OvfCount++;
    }
}

/**
 * @brief  Start the peripheral that was configured to generate the condition that to be captured by Timer2.
 * @param  None
 * @retval None
 */
static void Tmr2CaptureCondStart(void)
{
#if (EXAMPLE_FUNC == FUNC_CAPTURE_EVENT)
    /* In this example: press BSP key K4 to generate the event for TIEMR2 capturing. */
#else
    /* Make falling/rising edge on pin TIM2_<t>_TRIGx. */
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
