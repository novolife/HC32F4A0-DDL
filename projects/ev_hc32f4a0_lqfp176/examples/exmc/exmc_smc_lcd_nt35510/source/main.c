/**
 *******************************************************************************
 * @file  exmc/exmc_smc_lcd_nt35510/source/main.c
 * @brief This example demonstrates LCD function.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2023-01-15       CDT             First version
   2023-09-30       CDT             Change EXCLK freqeuncy to 30MHz
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
 * @addtogroup EXMC_SMC_LCD_NT35510
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/**
 * @brief point
 */
typedef struct {
    uint16_t u16X;
    uint16_t u16Y;
} stc_touchpad_point_t;

/**
 * @brief window
 */
typedef struct {
    uint16_t u16X1;
    uint16_t u16Y1;
    uint16_t u16X2;
    uint16_t u16Y2;
} stc_touchpad_window_t;

/**
 * @brief touch data definition
 */
typedef struct {
    stc_touchpad_point_t stcPoint;
    en_flag_status_t enPointPress;
} stc_touchpad_data_t;

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static stc_touchpad_data_t m_stcTouchData;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Return true is the touchpad is pressed
 * @param  None
 * @retval Press state
 */
static bool TOUCHPAD_IsPressed(void)
{
    uint8_t u8Tmp;
    uint8_t u8Status;
    bool bPressed = false;

    BSP_GT9XX_REG_Read(GT9XX_TOUCH_STATUS, &u8Status, 1UL);
    if ((u8Status & 0x80U) != 0U) {
        u8Tmp = 0U;
        BSP_GT9XX_REG_Write(GT9XX_TOUCH_STATUS, &u8Tmp, 1U);

        if ((0U < (u8Status & 0x0FU)) && ((u8Status & 0x0FU) < 6U)) {
            bPressed = true;
        }
    }

    return bPressed;
}

/**
 * @brief  Get touch data.
 * @param  [out] pstcData               Pointer to a @ref stc_touchpad_data_t structure.
 * @retval None
 */
static void TOUCHPAD_Read(stc_touchpad_data_t *pstcData)
{
    static uint16_t u16LastX = 0U;
    static uint16_t u16LastY = 0U;

    /*Save the pressed coordinates and the state*/
    if (TOUCHPAD_IsPressed()) {
        BSP_GT9XX_GetXY(GT9XX_POINT1, &u16LastX, &u16LastY);
        pstcData->enPointPress = SET;
    } else {
        pstcData->enPointPress = RESET;
    }

    /*Set the last pressed coordinates*/
    pstcData->stcPoint.u16X = u16LastX;
    pstcData->stcPoint.u16Y = u16LastY;
}

/**
 * @brief  Check if a point is on an window
 * @param  [in] pstcWin                 Pointer to a @ref stc_touchpad_window_t structure.
 * @param  [in] pstcPoint               Pointer to a @ref stc_touchpad_point_t structure.
 * @retval bool:
 *           - true:                    The point is in the area.
 *           - false:                   The point is out the area.
 */
bool TOUCHPAD_IsPointOn(const stc_touchpad_window_t *pstcWin, const stc_touchpad_point_t *pstcPoint)
{
    bool bIsPointOnWin = false;

    if ((pstcPoint->u16X >= pstcWin->u16X1 && pstcPoint->u16X <= pstcWin->u16X2) && \
        (pstcPoint->u16Y >= pstcWin->u16Y1 && pstcPoint->u16Y <= pstcWin->u16Y2)) {
        bIsPointOnWin = true;
    }

    return bIsPointOnWin;
}

/**
 * @brief  Main function of LCD project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint8_t i;
    uint16_t u16Width;
    uint16_t u16Height;
    bool bIsPointOnWin;
    const uint8_t u8RectangleCount = 3U;
    uint16_t u16WinSize;
    stc_touchpad_window_t stcWin;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* Initialize system clock: */
    BSP_CLK_Init();

    /* EXCLK 30MHz */
    CLK_SetClockDiv(CLK_BUS_EXCLK, CLK_EXCLK_DIV8);

    BSP_IO_Init();
    BSP_LCD_IO_Init();

    /* Initialize LCD touch pad */
    BSP_GT9XX_Init();

    /* HW Reset LCD */
    BSP_LCD_RSTCmd(EIO_PIN_RESET); /* RST# to low */
    DDL_DelayMS(50UL);
    BSP_LCD_RSTCmd(EIO_PIN_SET);  /* RST# to high */
    DDL_DelayMS(50UL);

    /* Initialize NT35510 LCD */
    BSP_NT35510_Init();

    /* Clear LCD screen */
    BSP_NT35510_Clear(LCD_COLOR_BLACK);

    /* Turn on LCD backlight */
    BSP_LCD_BKLCmd(EIO_PIN_SET);

    /* Set LCD cursor */
    BSP_NT35510_SetCursor(0U, 0U);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    u16Width = BSP_NT35510_GetPixelWidth();
    u16Height = BSP_NT35510_GetPixelHeight();

    for (;;) {
        for (i = 0U; i < u8RectangleCount; i++) {
            bIsPointOnWin = false;
            u16WinSize = u16Width / u8RectangleCount;

            stcWin.u16X1 = (i * u16WinSize);
            stcWin.u16Y1 = (u16Height / 2U) - (u16WinSize / 2U);
            stcWin.u16X2 = stcWin.u16X1 + u16WinSize - 1U;
            stcWin.u16Y2 = (u16Height / 2U) + (u16WinSize / 2U) - 1U;
            BSP_NT35510_DrawRectangle(stcWin.u16X1, stcWin.u16Y1, stcWin.u16X2, stcWin.u16Y2, LCD_COLOR_GREEN);

            do {
                (void)memset(&m_stcTouchData, 0, sizeof(m_stcTouchData));
                TOUCHPAD_Read(&m_stcTouchData);
                if (m_stcTouchData.enPointPress == SET) {
                    bIsPointOnWin = TOUCHPAD_IsPointOn(&stcWin, &m_stcTouchData.stcPoint);
                }
            } while (false == bIsPointOnWin);

            BSP_NT35510_Clear(LCD_COLOR_BLACK);
        }

        BSP_NT35510_DrawCircle(u16Width / 2U, u16Height / 2U, u16Width / 4U, LCD_COLOR_RED);
        DDL_DelayMS(1000UL);

        BSP_NT35510_Clear(LCD_COLOR_RED);
        DDL_DelayMS(1000UL);

        BSP_NT35510_Clear(LCD_COLOR_GREEN);
        DDL_DelayMS(1000UL);

        BSP_NT35510_Clear(LCD_COLOR_BLUE);
        DDL_DelayMS(1000UL);

        /* Clear LCD screen */
        BSP_NT35510_Clear(LCD_COLOR_BLACK);
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
