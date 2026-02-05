/**
 *******************************************************************************
 * @file  rtc/rtc_calendar/source/main.c
 * @brief Main program of RTC Calendar for the Device Driver Library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2023-09-30       CDT             Optimize RTC init sequence
                                    Modify the judgment condition for RTC operation status
   2024-11-08       CDT             Add XTAL32 clock source selection for RTC
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
 * @addtogroup RTC_Calendar
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

/* BACKUP REG: 96~127 */
#define RTC_BACKUP_REG_START            (96U)
#define RTC_BACKUP_DATA_SIZE            (32U)

/* RTC clock source selection */
#define RTC_CLK_SRC_SEL                 (RTC_CLK_SRC_LRC)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
static const uint8_t u8BackupWriteData[RTC_BACKUP_DATA_SIZE] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static __IO uint8_t u8SecIntFlag = 0U;
static uint8_t u8BackupReadData[RTC_BACKUP_DATA_SIZE];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  RTC period interrupt callback function.
 * @param  None
 * @retval None
 */
static void RTC_Period_IrqCallback(void)
{
    u8SecIntFlag = 1U;
    RTC_ClearStatus(RTC_FLAG_PERIOD);
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
 * @brief  Write backup registers.
 * @param  None
 * @retval None
 */
static void RTC_WriteBackupReg(void)
{
    uint8_t u8Num;

    for (u8Num = 0U; u8Num < RTC_BACKUP_DATA_SIZE; u8Num++) {
        PWC_BKR_Write(RTC_BACKUP_REG_START + u8Num, u8BackupWriteData[u8Num]);
    }
}

/**
 * @brief  Check backup registers.
 * @param  None
 * @retval None
 */
static int32_t RTC_CheckBackupReg(void)
{
    uint8_t u8Num;
    int32_t i32Ret = LL_OK;

    for (u8Num = 0U; u8Num < RTC_BACKUP_DATA_SIZE; u8Num++) {
        u8BackupReadData[u8Num] = PWC_BKR_Read(RTC_BACKUP_REG_START + u8Num);
    }
    for (u8Num = 0U; u8Num < RTC_BACKUP_DATA_SIZE; u8Num++) {
        if (u8BackupWriteData[u8Num] != u8BackupReadData[u8Num]) {
            i32Ret = LL_ERR;
            break;
        }
    }

    return i32Ret;
}

/**
 * @brief  RTC configuration.
 * @param  None
 * @retval None
 */
static void RTC_Config(void)
{
    int32_t i32Ret;
    stc_rtc_init_t stcRtcInit;
    stc_irq_signin_config_t stcIrqSignConfig;

    /* RTC period interrupt configure */
    stcIrqSignConfig.enIntSrc    = INT_SRC_RTC_PRD;
    stcIrqSignConfig.enIRQn      = INT052_IRQn;
    stcIrqSignConfig.pfnCallback = &RTC_Period_IrqCallback;
    (void)INTC_IrqSignOut(stcIrqSignConfig.enIRQn);
    i32Ret = INTC_IrqSignIn(&stcIrqSignConfig);
    if (LL_OK != i32Ret) {
        /* check parameter */
        for (;;) {
        }
    }

    /* Clear pending */
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    /* Set priority */
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

    /* Check sequence flag */
    if (LL_OK == RTC_CheckBackupReg()) {
        /* Enter read/write mode */
        if (LL_OK == RTC_EnterRwMode()) {
            /* Exit read/write mode */
            if (LL_OK == RTC_ExitRwMode()) {
                return;
            }
        }
    }

    /* Reset the VBAT area */
    PWC_VBAT_Reset();
#if RTC_CLK_SRC_SEL == RTC_CLK_SRC_XTAL32
    if (LL_OK != BSP_XTAL32_Init()) {
        DDL_Printf("XTAL32 init failed!\r\n");
        return;
    }
#endif
    /* Reset RTC counter */
    if (LL_ERR_TIMEOUT == RTC_DeInit()) {
        DDL_Printf("Reset RTC failed!\r\n");
    } else {
        /* Stop RTC */
        RTC_Cmd(DISABLE);
        /* Configure structure initialization */
        (void)RTC_StructInit(&stcRtcInit);

        /* Configuration RTC structure */
        stcRtcInit.u8ClockSrc   = RTC_CLK_SRC_SEL;
        stcRtcInit.u8HourFormat = RTC_HOUR_FMT_24H;
        stcRtcInit.u8IntPeriod  = RTC_INT_PERIOD_PER_SEC;
        (void)RTC_Init(&stcRtcInit);

        /* Update date and time */
        RTC_CalendarConfig();
        /* Enable period interrupt */
        RTC_ClearStatus(RTC_FLAG_CLR_ALL);
        RTC_IntCmd(RTC_INT_PERIOD, ENABLE);
        /* Startup RTC count */
        RTC_Cmd(ENABLE);
        /* Write sequence flag */
        RTC_WriteBackupReg();
    }
}

/**
 * @brief  Main function of RTC Calendar.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_rtc_date_t stcCurrentDate;
    stc_rtc_time_t stcCurrentTime;

    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure clock */
    BSP_CLK_Init();
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
        if (1U == u8SecIntFlag) {
            u8SecIntFlag = 0U;
            BSP_LED_Toggle(LED_RED);
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
