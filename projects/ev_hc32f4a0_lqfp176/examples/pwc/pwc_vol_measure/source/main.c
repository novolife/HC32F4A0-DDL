/**
 *******************************************************************************
 * @file  pwc/pwc_vol_measure/source/main.c
 * @brief Main program of PWC voltage measure for the Device Driver Library.
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
 * @addtogroup PWC_Voltage_Measure
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* ADC unit instance for this example */
#define ADC_UNIT                        (CM_ADC1)
#define ADC_PERIPH_CLK                  (FCG3_PERIPH_ADC1)
#define ADC_CH                          (ADC_EXT_CH)

/* ADC channel sampling time */
#define ADC_SA_SAMPLE_TIME              (25U)

/* ADC sequence to be used. */
#define ADC_SEQ                         (ADC_SEQ_A)
/* Flag of conversion end. */
#define ADC_EOC_FLAG                    (ADC_FLAG_EOCA)

/* ADC reference voltage. The voltage of pin VREFH. */
#define ADC_VREF                        (3.245F)

/* ADC accuracy(according to the resolution of ADC). */
#define ADC_ACCURACY                    (1UL << 12U)

/* Calculate the voltage(mV). */
#define ADC_CAL_VOL(adcVal)             (uint16_t)((((float32_t)(adcVal) * ADC_VREF) / ((float32_t)ADC_ACCURACY)) * 1000.F)

/* Timeout value. */
#define ADC_TIMEOUT_VAL                 (1000U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint16_t m_u16AdcValue;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  ADC configuration.
 * @param  None
 * @retval None
 */
static void AdcConfig(void)
{
    stc_adc_init_t stcAdcInit;

    /* Specify the clock source of ADC */
    CLK_SetPeriClockSrc(CLK_PERIPHCLK_PCLK);

    /* Enable ADC peripheral clock. */
    FCG_Fcg3PeriphClockCmd(ADC_PERIPH_CLK, ENABLE);

    /* Modify the default value depends on the application. Not needed here. */
    (void)ADC_StructInit(&stcAdcInit);

    /* Initializes ADC. */
    (void)ADC_Init(ADC_UNIT, &stcAdcInit);

    /* ADC channel configuration. */
    /* Set the ADC pin to analog input mode. Not needed here. */
    /* Enable ADC channels. Call ADC_ChCmd() again to enable more channels if needed. */
    ADC_ChCmd(ADC_UNIT, ADC_SEQ, ADC_CH, ENABLE);
    /* Set the analog source of extended channel. */
    ADC_SetExtChSrc(ADC_UNIT, ADC_EXTCH_INTERN_ANALOG_SRC);
    /* Set the sampling time of the channel, if needed. */
    ADC_SetSampleTime(ADC_UNIT, ADC_CH, ADC_SA_SAMPLE_TIME);

    /* Conversion data average calculation function, if needed.
       Call ADC_ConvDataAverageChCmd() again to enable more average channels if needed. */
    ADC_ConvDataAverageConfig(ADC_UNIT, ADC_AVG_CNT8);
    ADC_ConvDataAverageChCmd(ADC_UNIT, ADC_CH, ENABLE);
}

/**
 * @brief  Use ADC in polling mode.
 * @param  None
 * @retval None
 */
static void AdcPolling(void)
{
    int32_t iRet = LL_ERR;
    __IO uint32_t u32TimeCount = 0UL;

    /* Can ONLY start sequence A conversion.
       Sequence B needs hardware trigger to start conversion. */
    ADC_Start(ADC_UNIT);
    do {
        if (ADC_GetStatus(ADC_UNIT, ADC_EOC_FLAG) == SET) {
            ADC_ClearStatus(ADC_UNIT, ADC_EOC_FLAG);
            iRet = LL_OK;
            break;
        }
    } while (u32TimeCount++ < ADC_TIMEOUT_VAL);

    if (iRet == LL_OK) {
        /* Get any ADC value of sequence A channel that needed. */
        m_u16AdcValue = ADC_GetValue(ADC_UNIT, ADC_CH);

    } else {
        ADC_Stop(ADC_UNIT);
    }
}

/**
 * @brief  Main function of PWC voltage measure.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Register write enable for some required peripherals. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* Configure clock */
    BSP_CLK_Init();
    /* Reset VBAT area */
    PWC_VBAT_Reset();
    /* Configure BSP */
    BSP_IO_Init();
    BSP_LED_Init();
    /* Configure UART */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configures ADC. */
    AdcConfig();

    /* Get the internal reference voltage. */
    PWC_VBAT_MonitorCmd(ENABLE);
    PWC_VBAT_VoltageDivMonitorCmd(DISABLE);
    PWC_SetPowerMonitorVoltageSrc(PWC_PWR_MON_IREF);
    PWC_PowerMonitorCmd(ENABLE);
    /* Delay 50us is needed. */
    DDL_DelayMS(1U);
    AdcPolling();
    DDL_Printf("Internal reference voltage: adc value is %u, voltage is %u mV\r\n", m_u16AdcValue, ADC_CAL_VOL(m_u16AdcValue));

    /* Get the voltage of VBAT. */
    PWC_VBAT_VoltageDivMonitorCmd(ENABLE);
    PWC_SetPowerMonitorVoltageSrc(PWC_PWR_MON_VBAT_DIV2);
    /* Delay 50us is needed. */
    DDL_DelayMS(1U);
    AdcPolling();
    DDL_Printf("VBAT: adc value is %u, voltage is %u mV\r\n", m_u16AdcValue * 2U, ADC_CAL_VOL(m_u16AdcValue) * 2U);

    /* VBAT voltage measure. */
    PWC_VBAT_VoltageDivMonitorCmd(DISABLE);
    PWC_VBAT_SetMonitorVoltage(PWC_VBAT_REF_VOL_2P1V);
    /* Delay 50us is needed. */
    DDL_DelayMS(1U);
    /* Register write protected for some required peripherals. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    for (;;) {
        AdcPolling();
        if (RESET == PWC_VBAT_GetVoltageStatus()) {
            /* Vbat > VbatRef */
            BSP_LED_On(LED_BLUE);
            BSP_LED_Off(LED_RED);
        } else {
            /* Vbat < VbatRef */
            BSP_LED_On(LED_RED);
            BSP_LED_Off(LED_BLUE);
        }
        DDL_DelayMS(100UL);
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
