/**
 *******************************************************************************
 * @file  adc/adc_extended_channel/source/main.c
 * @brief Main program ADC extended channel for the Device Driver Library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2023-09-30       CDT             Set XTAL as system clock source
                                    Modify printf baudrate
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
 * @addtogroup ADC_Extended_Channel
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* ADC unit instance for this example. */
#define ADC_UNIT                        (CM_ADC1)
#define ADC_PERIPH_CLK                  (FCG3_PERIPH_ADC1)

/* Selects ADC channels that needed. */
#define ADC_CH                          (ADC_EXT_CH)

/* ADC sequence to be used. */
#define ADC_SEQ                         (ADC_SEQ_A)
/* Flag of conversion end. */
#define ADC_EOC_FLAG                    (ADC_FLAG_EOCA)

/* ADC reference voltage. The voltage of pin VREFH. */
#define ADC_VREF                        (3.3F)

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
static void SystemClockConfig(void);

static void AdcConfig(void);
static void AdcInitConfig(void);
static void AdcExtChSrcConfig(void);
static void AdcPolling(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint16_t m_u16AdcValue;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of adc_extended_channel project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected. */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);
    /* System clock config */
    SystemClockConfig();
    /* Initializes UART for debug printing. Baudrate is 115200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configures ADC. */
    AdcConfig();
    /* MCU Peripheral registers write protected. */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU);

    /***************** Configuration end, application start **************/

    for (;;) {
        /* Get the internal reference voltage. */
        LL_PERIPH_WE(LL_PERIPH_PWC_CLK_RMU);
        PWC_VBAT_MonitorCmd(DISABLE);
        PWC_VBAT_VoltageDivMonitorCmd(DISABLE);
        PWC_SetPowerMonitorVoltageSrc(PWC_PWR_MON_IREF);
        LL_PERIPH_WP(LL_PERIPH_PWC_CLK_RMU);
        /* Delay 50us is needed. */
        DDL_DelayUS(100U);
        AdcPolling();
        DDL_Printf("Internal reference voltage is adc value is %u, voltage is %u mV\r\n", \
                   m_u16AdcValue, ADC_CAL_VOL(m_u16AdcValue));

        /* Get the half voltage of VBAT. */
        LL_PERIPH_WE(LL_PERIPH_PWC_CLK_RMU);
        PWC_VBAT_MonitorCmd(ENABLE);
        PWC_VBAT_VoltageDivMonitorCmd(ENABLE);
        PWC_SetPowerMonitorVoltageSrc(PWC_PWR_MON_VBAT_DIV2);
        LL_PERIPH_WP(LL_PERIPH_PWC_CLK_RMU);
        /* Delay 50us is needed. */
        DDL_DelayUS(100U);
        AdcPolling();
        DDL_Printf("VBAT/2: adc value is %u, voltage is %u mV\r\n", \
                   m_u16AdcValue, ADC_CAL_VOL(m_u16AdcValue));

        DDL_DelayMS(500UL);
    }
}

/**
 * @brief  Set XTAL as system clock source.
 * @param  None
 * @retval None
 */
static void SystemClockConfig(void)
{
    stc_clock_xtal_init_t stcXtalInit;

    /* XTAL config */
    GPIO_AnalogCmd(BSP_XTAL_PORT, BSP_XTAL_PIN, ENABLE);
    (void)CLK_XtalStructInit(&stcXtalInit);
    /* Config XTAL and Enable XTAL */
    stcXtalInit.u8State = CLK_XTAL_ON;
    stcXtalInit.u8Mode = CLK_XTAL_MD_OSC;
    stcXtalInit.u8Drv = CLK_XTAL_DRV_ULOW;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_2MS;
    (void)CLK_XtalInit(&stcXtalInit);
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_XTAL);
}

/**
 * @brief  ADC configuration.
 * @param  None
 * @retval None
 */
static void AdcConfig(void)
{
    AdcInitConfig();
    AdcExtChSrcConfig();
}

/**
 * @brief  Initializes ADC.
 * @param  None
 * @retval None
 */
static void AdcInitConfig(void)
{
    stc_adc_init_t stcAdcInit;

    /* 1. Enable ADC peripheral clock. */
    FCG_Fcg3PeriphClockCmd(ADC_PERIPH_CLK, ENABLE);

    /* 2. Modify the default value depends on the application. Not needed here. */
    (void)ADC_StructInit(&stcAdcInit);

    /* 3. Initializes ADC. */
    (void)ADC_Init(ADC_UNIT, &stcAdcInit);

    /* 4. ADC channel configuration. */
    /* 4.1 Set the ADC pin to analog input mode, not needed here. */
    /* 4.2 Enable ADC channels. */
    ADC_ChCmd(ADC_UNIT, ADC_SEQ, ADC_CH, ENABLE);

    /* 5. Conversion data average calculation function, if needed.
          Call ADC_ConvDataAverageChCmd() again to enable more average channels if needed. */
    ADC_ConvDataAverageConfig(ADC_UNIT, ADC_AVG_CNT8);
    ADC_ConvDataAverageChCmd(ADC_UNIT, ADC_CH, ENABLE);
}

/**
 * @brief  Configures the analog input source for the extended channel.
 * @param  None
 * @retval None
 */
static void AdcExtChSrcConfig(void)
{
    PWC_PowerMonitorCmd(ENABLE);
    ADC_SetExtChSrc(ADC_UNIT, ADC_EXTCH_INTERN_ANALOG_SRC);
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
        m_u16AdcValue = ADC_GetValue(ADC_UNIT, ADC_CH);
    } else {
        ADC_Stop(ADC_UNIT);
        DDL_Printf("ADC exception.\r\n");
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
