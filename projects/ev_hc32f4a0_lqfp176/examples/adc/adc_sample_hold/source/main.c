/**
 *******************************************************************************
 * @file  adc/adc_sample_hold/source/main.c
 * @brief Main program ADC sample-hold for the Device Driver Library.
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
 * @addtogroup ADC_Sample_Hold
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* ADC unit instance for this example. Only ADC1 supports sample-hold. */
#define ADC_UNIT                        (CM_ADC1)
#define ADC_PERIPH_CLK                  (FCG3_PERIPH_ADC1)
#define SH_PERIPH_CLK                   (FCG3_PERIPH_CMBIAS)

/* Sample-hold channels(ADC_CH0, ADC_CH1, ADC_CH2) */
#define ADC_SH_CH0                      (ADC_CH0)
#define ADC_SH_CH0_PORT                 (GPIO_PORT_A)
#define ADC_SH_CH0_PIN                  (GPIO_PIN_00)

#define ADC_SH_CH1                      (ADC_CH1)
#define ADC_SH_CH1_PORT                 (GPIO_PORT_A)
#define ADC_SH_CH1_PIN                  (GPIO_PIN_01)

#define ADC_SH_CH2                      (ADC_CH2)
#define ADC_SH_CH2_PORT                 (GPIO_PORT_A)
#define ADC_SH_CH2_PIN                  (GPIO_PIN_02)

/* ADC sequence to be used. */
#define ADC_SEQ                         (ADC_SEQ_A)
/* Flag of conversion end. */
#define ADC_EOC_FLAG                    (ADC_FLAG_EOCA)

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
static void AdcSetPinAnalogMode(void);
static void AdcSampleHoldConfig(void);
static void AdcPolling(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of adc_sample_hold project
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
        AdcPolling();
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
    AdcSampleHoldConfig();
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
    /* 4.1 Set the ADC pin to analog input mode. */
    AdcSetPinAnalogMode();
    /* 4.2 Enable ADC channels. */
    ADC_ChCmd(ADC_UNIT, ADC_SEQ_A, ADC_SH_CH0, ENABLE);
    ADC_ChCmd(ADC_UNIT, ADC_SEQ_A, ADC_SH_CH1, ENABLE);
    ADC_ChCmd(ADC_UNIT, ADC_SEQ_A, ADC_SH_CH2, ENABLE);
}

/**
 * @brief  Set specified ADC pin to analog mode.
 * @param  None
 * @retval None
 */
static void AdcSetPinAnalogMode(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;
    (void)GPIO_Init(ADC_SH_CH0_PORT, ADC_SH_CH0_PIN, &stcGpioInit);
    (void)GPIO_Init(ADC_SH_CH1_PORT, ADC_SH_CH1_PIN, &stcGpioInit);
    (void)GPIO_Init(ADC_SH_CH2_PORT, ADC_SH_CH2_PIN, &stcGpioInit);
}

/**
 * @brief  Sample-hold configuration.
 * @param  None
 * @retval None
 */
static void AdcSampleHoldConfig(void)
{
    /* 1. Enable sample-hold peripheral clock. */
    FCG_Fcg3PeriphClockCmd(SH_PERIPH_CLK, ENABLE);

    /* 2. Delay 2us is needed by sample-hold. */
    DDL_DelayUS(5U);

    /**
     * 3. Specify the sampling time of sample-hold. The time should longer than 0.4us.
     *    In this example, PCLK2 is the clock of ADC analog circuit which is 8MHz.
     *    0.4us = (1 / 8) * 3.2.
     */
    ADC_SH_SetSampleTime(ADC_UNIT, 10U);
    /* 4. Enable sample hold channel. */
    ADC_SH_ChCmd(ADC_UNIT, ADC_SH_CH0, ENABLE);
    ADC_SH_ChCmd(ADC_UNIT, ADC_SH_CH1, ENABLE);
    ADC_SH_ChCmd(ADC_UNIT, ADC_SH_CH2, ENABLE);
}

/**
 * @brief  Use ADC in polling mode.
 * @param  None
 * @retval None
 */
static void AdcPolling(void)
{
    uint16_t u16AdcValueShCh0;
    uint16_t u16AdcValueShCh1;
    uint16_t u16AdcValueShCh2;
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
        u16AdcValueShCh0 = ADC_GetValue(ADC_UNIT, ADC_SH_CH0);
        u16AdcValueShCh1 = ADC_GetValue(ADC_UNIT, ADC_SH_CH1);
        u16AdcValueShCh2 = ADC_GetValue(ADC_UNIT, ADC_SH_CH2);
        DDL_Printf("CH0 adc value: %u\r\nCH1 adc value: %u\r\nCH2 adc value: %u\r\n", \
                   u16AdcValueShCh0, u16AdcValueShCh1, u16AdcValueShCh2);
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
