/**
 *******************************************************************************
 * @file  rtc/rtc_intrusion_detect/source/main.c
 * @brief Main program of RTC Intrusion Detect for the Device Driver Library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-10-31       CDT             Add configuration of XTAL32 IO as analog function
   2023-09-30       CDT             Optimize RTC init sequence
                                    Replace XTAL32_ClkInit to BSP_XTAL32_Init
   2024-11-08       CDT             Place BSP_XTAL32_Init function after PWC_VBAT_Reset function
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
 * @addtogroup RTC_Intrusion_Detect
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
/* RTC Intrusion channel 1 Port/Pin definition */
#define RTC_INTRU_CHAN_1_PORT                   (GPIO_PORT_I)
#define RTC_INTRU_CHAN_1_PIN                    (GPIO_PIN_08)

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE                       (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                                 LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP                       (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)
#define RTC_INTRU_BACKUP_ADDR                   (0x10U)
#define RTC_INTRU_BACKUP_FLAG                   (0xA0U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static __IO uint8_t u8SecIntFlag = 0U;
static __IO uint8_t u8IntruIntFlag = 0U;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  RTC period interrupt callback function.
 * @param  None
 * @retval None
 */
void RTC_Period_IrqHandler(void)
{
    u8SecIntFlag = 1U;
    RTC_ClearStatus(RTC_FLAG_PERIOD);
}

/**
 * @brief  RTC intrusion channel 1 interrupt callback function.
 * @param  None
 * @retval None
 */
void RTC_TimeStamp1_IrqHandler(void)
{
    u8IntruIntFlag = 1U;
    RTC_ClearStatus(RTC_FLAG_INTRU_CH1);
}

/**
 * @brief  RTC calendar configuration.
 * @param  None
 * @retval None
 */
static void RTC_CalendarConfig(void)
{
    stc_rtc_date_t stcRtcDate;
    stc_rtc_time_t stcRtcTime;

    /* Date configuration */
    stcRtcDate.u8Year    = 20U;
    stcRtcDate.u8Month   = RTC_MONTH_JANUARY;
    stcRtcDate.u8Day     = 1U;
    stcRtcDate.u8Weekday = RTC_WEEKDAY_WEDNESDAY;

    /* Time configuration */
    stcRtcTime.u8Hour   = 23U;
    stcRtcTime.u8Minute = 59U;
    stcRtcTime.u8Second = 55U;
    stcRtcTime.u8AmPm   = RTC_HOUR_12H_AM;

    if (LL_OK != RTC_SetDate(RTC_DATA_FMT_DEC, &stcRtcDate)) {
        DDL_Printf("Set Date failed!\r\n");
    }

    if (LL_OK != RTC_SetTime(RTC_DATA_FMT_DEC, &stcRtcTime)) {
        DDL_Printf("Set Time failed!\r\n");
    }
}

/**
 * @brief  RTC display weekday.
 * @param  [in] u8Weekday               Weekday
 *         This parameter can be one of the following values:
 *           @arg RTC_WEEKDAY_SUNDAY:     Sunday
 *           @arg RTC_WEEKDAY_MONDAY:     Monday
 *           @arg RTC_WEEKDAY_TUESDAY:    Tuesday
 *           @arg RTC_WEEKDAY_WEDNESDAY:  Wednesday
 *           @arg RTC_WEEKDAY_THURSDAY:   Thursday
 *           @arg RTC_WEEKDAY_FRIDAY:     Friday
 *           @arg RTC_WEEKDAY_SATURDAY:   Saturday
 * @retval None
 */
static void RTC_DisplayWeekday(uint8_t u8Weekday)
{
    switch (u8Weekday) {
        case RTC_WEEKDAY_SUNDAY:
            DDL_Printf("Sunday\r\n");
            break;
        case RTC_WEEKDAY_MONDAY:
            DDL_Printf("Monday\r\n");
            break;
        case RTC_WEEKDAY_TUESDAY:
            DDL_Printf("Tuesday\r\n");
            break;
        case RTC_WEEKDAY_WEDNESDAY:
            DDL_Printf("Wednesday\r\n");
            break;
        case RTC_WEEKDAY_THURSDAY:
            DDL_Printf("Thursday\r\n");
            break;
        case RTC_WEEKDAY_FRIDAY:
            DDL_Printf("Friday\r\n");
            break;
        case RTC_WEEKDAY_SATURDAY:
            DDL_Printf("Saturday\r\n");
            break;
        default:
            break;
    }
}

/**
 * @brief  RTC configuration.
 * @param  None
 * @retval None
 */
static void RTC_Config(void)
{
    stc_rtc_init_t stcRtcInit;
    stc_rtc_intrusion_t stcRtcIntrusion;
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_ON;
    (void)GPIO_Init(RTC_INTRU_CHAN_1_PORT, RTC_INTRU_CHAN_1_PIN, &stcGpioInit);

    /* Reset RTC counter */
    if (LL_ERR_TIMEOUT == RTC_DeInit()) {
        DDL_Printf("Reset RTC failed!\r\n");
    } else {
        /* Stop RTC */
        RTC_Cmd(DISABLE);
        /* Configure structure initialization */
        (void)RTC_StructInit(&stcRtcInit);

        /* Configuration RTC structure */
        stcRtcInit.u8ClockSrc   = RTC_CLK_SRC_XTAL32;
        stcRtcInit.u8HourFormat = RTC_HOUR_FMT_24H;
        stcRtcInit.u8IntPeriod  = RTC_INT_PERIOD_PER_SEC;
        (void)RTC_Init(&stcRtcInit);

        /* Configuration intrusion function */
        stcRtcIntrusion.u8Timestamp      = RTC_INTRU_TS_ENABLE;
        stcRtcIntrusion.u8ResetBackupReg = RTC_INTRU_RST_BACKUP_REG_ENABLE;
        stcRtcIntrusion.u8Filter         = RTC_INTRU_FILTER_THREE_TIME;
        stcRtcIntrusion.u8TriggerEdge    = RTC_INTRU_TRIG_EDGE_FALLING;
        (void)RTC_INTRU_Init(RTC_INTRU_CH1, &stcRtcIntrusion);
        RTC_INTRU_Cmd(RTC_INTRU_CH1, ENABLE);

        /* Update date and time */
        RTC_CalendarConfig();
        /* RTC period and intrusion interrupt configure */
        (void)INTC_ShareIrqCmd(INT_SRC_RTC_TP, ENABLE);
        (void)INTC_ShareIrqCmd(INT_SRC_RTC_PRD, ENABLE);
        /* Clear pending */
        NVIC_ClearPendingIRQ(INT131_IRQn);
        /* Set priority */
        NVIC_SetPriority(INT131_IRQn, DDL_IRQ_PRIO_DEFAULT);
        /* Enable NVIC */
        NVIC_EnableIRQ(INT131_IRQn);
        /* Enable period and intrusion interrupt */
        RTC_ClearStatus(RTC_FLAG_CLR_ALL);
        RTC_IntCmd((RTC_INT_PERIOD | RTC_INT_INTRU_CH1), ENABLE);
        /* Startup RTC count */
        RTC_Cmd(ENABLE);
    }
}

/**
 * @brief  Main function of RTC Intrusion Detect.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_rtc_date_t stcCurrentDate;
    stc_rtc_time_t stcCurrentTime;
    stc_rtc_timestamp_t stcTimestamp;

    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure clock */
    BSP_CLK_Init();
    /* Reset the VBAT area */
    PWC_VBAT_Reset();
    BSP_XTAL32_Init();
    /* Intrusion mark */
    PWC_BKR_Write(RTC_INTRU_BACKUP_ADDR, RTC_INTRU_BACKUP_FLAG);
    /* Configure BSP */
    BSP_IO_Init();
    BSP_LED_Init();
    /* Configure UART */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Configure RTC */
    RTC_Config();
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);

    for (;;) {
        if (1U == u8IntruIntFlag) {
            u8IntruIntFlag = 0U;
            /* Get intrusion timestamp */
            if (LL_OK == RTC_INTRU_GetTimestamp(RTC_DATA_FMT_BCD, &stcTimestamp)) {
                DDL_Printf("RTCIC1 (PI8) trigger intrusion event!\r\n");
                DDL_Printf("Timestamp %02x/%02x %02x:%02x:%02x \r\n", stcTimestamp.u8Month, stcTimestamp.u8Day,
                           stcTimestamp.stcTime.u8Hour, stcTimestamp.stcTime.u8Minute,
                           stcTimestamp.stcTime.u8Second);
            } else {
                DDL_Printf("Get intrusion timestamp failed!\r\n");
            }
        } else if (1U == u8SecIntFlag) {
            u8SecIntFlag = 0U;
            /* Intrusion LED */
            if (0U == PWC_BKR_Read(RTC_INTRU_BACKUP_ADDR)) {
                BSP_LED_Toggle(LED_RED);
            }
            /* Get current date */
            if (LL_OK == RTC_GetDate(RTC_DATA_FMT_DEC, &stcCurrentDate)) {
                /* Get current time */
                if (LL_OK == RTC_GetTime(RTC_DATA_FMT_DEC, &stcCurrentTime)) {
                    /* Print current date and time */
                    DDL_Printf("20%02d/%02d/%02d %02d:%02d:%02d ", stcCurrentDate.u8Year, stcCurrentDate.u8Month,
                               stcCurrentDate.u8Day, stcCurrentTime.u8Hour,
                               stcCurrentTime.u8Minute, stcCurrentTime.u8Second);
                    RTC_DisplayWeekday(stcCurrentDate.u8Weekday);
                } else {
                    DDL_Printf("Get time failed!\r\n");
                }
            } else {
                DDL_Printf("Get date failed!\r\n");
            }
        } else {
            /* Reserverd */
        }
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
