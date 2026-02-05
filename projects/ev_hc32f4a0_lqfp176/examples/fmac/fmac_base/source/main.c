/**
 *******************************************************************************
 * @file  fmac/fmac_base/source/main.c
 * @brief Main program FMAC for the Device Driver Library.
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
 * @addtogroup FMAC_Base
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define FMACx                   (CM_FMAC1)
#define FIR_STAGE               (FMAC_FIR_STAGE_2)
#define FIR_LEN                 (FMAC_FIR_STAGE_2 + 1U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void FMACInitConfig(void);
static int32_t SW_FIR_Filter(int16_t *i16h, int32_t *i32x);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static int32_t m_ai32X[] = {100, 200, 300};
static int16_t m_ai16H[] = {2, 6, 2};

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Initializes FMAC.
 * @param  None
 * @retval None
 */
static void FMACInitConfig(void)
{
    stc_fmac_init_t stcFmacInit;

    /* ENABLE FMAC1 peripheral clock. */
    FCG_Fcg1PeriphClockCmd(PWC_FCG1_FMAC1, ENABLE);
    /* ENABLE FMAC1 */
    FMAC_Cmd(FMACx, ENABLE);

    /* FMAC function initialize */
    (void)FMAC_StructInit(&stcFmacInit);
    stcFmacInit.pi16Factor = m_ai16H;
    stcFmacInit.u32Stage   = FIR_STAGE;
    stcFmacInit.u32Shift   = FMAC_FIR_SHIFT_0BIT;
    stcFmacInit.u32IntCmd  = FMAC_INT_DISABLE;
    (void)FMAC_Init(FMACx, &stcFmacInit);
}

/**
 * @brief  Software FIR.
 * @param  [in] i16h: FIR factor
 * @param  [in] i32x: FIR input array
 * @retval i32result: software FIR result.
 */
static int32_t SW_FIR_Filter(int16_t *i16h, int32_t *i32x)
{
    uint8_t i;
    int32_t i32result = 0.0;

    for (i = 0U; i < FIR_LEN; i++) {
        i32result += i16h[i] * i32x[FIR_LEN - 1U - i];
    }
    return i32result;
}

/**
 * @brief  Main function of fmac_base project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_fmac_result_t stcResult;
    int32_t i32SWResult = 0;
    uint8_t i;
    /* Software FIR */
    i32SWResult = SW_FIR_Filter(m_ai16H, m_ai32X);
    /* Unlock peripherals or registers */
    LL_PERIPH_WE(LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_SRAM | LL_PERIPH_PWC_CLK_RMU);
    /* BSP clock initialize */
    BSP_CLK_Init();
    /* BSP expand IO initialize */
    BSP_IO_Init();
    /* BSP LED initialize */
    BSP_LED_Init();
    /* Configures FMAC. */
    FMACInitConfig();
    /* Initialize UART for debug printing. Baudrate is 115200. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* Lock peripherals or registers */
    LL_PERIPH_WP(LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_GPIO | LL_PERIPH_SRAM | LL_PERIPH_PWC_CLK_RMU);

    for (;;) {
        for (i = 0U; i < FIR_LEN; i++) {
            FMAC_FIRInput(FMACx, m_ai32X[i]);
        }
        /* Wait for calculation to complete */
        while (SET != FMAC_GetStatus(FMACx)) {
            ;
        }
        /* Get result and output it*/
        (void)FMAC_GetResult(FMACx, &stcResult);
        DDL_Printf("The result is %lu,%lu\r\n", stcResult.u32ResultHigh, stcResult.u32ResultLow);
        if (i32SWResult != stcResult.u32ResultLow) {
            BSP_LED_Toggle(LED_RED);
        } else {
            BSP_LED_Toggle(LED_BLUE);
        }
        DDL_DelayMS(200U);
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
