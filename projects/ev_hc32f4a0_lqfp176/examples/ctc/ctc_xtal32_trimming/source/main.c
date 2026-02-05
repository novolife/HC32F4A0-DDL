/**
 *******************************************************************************
 * @file  ctc/ctc_xtal32_trimming/source/main.c
 * @brief This example demonstrates CTC trimming by XTAL32 clock.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-10-31       CDT             Add configuration of XTAL and XTAL32 IO as analog function
   2023-09-30       CDT             Initialize XTAL32 using BSP_XTAL32_Init
   2024-11-08       CDT             Implement the weak function: CTC_Udf/Ovf_IrqHandler
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
 * @addtogroup CTC_XTAL32_Trimming
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/* Peripheral register WE/WP selection */
#define LL_PERIPH_SEL                   (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                                         LL_PERIPH_EFM | LL_PERIPH_SRAM)

/* MCO pin */
#define MCO_PORT                        (GPIO_PORT_A)
#define MCO_PIN                         (GPIO_PIN_08)
#define MCO_GPIO_FUNC                   (GPIO_FUNC_1)

/* CTC reference clock selection */
#define CTC_REF_CLK_SRC                 (CTC_REF_CLK_SRC_XTAL32)

/* CTC reference clock frequency */
#define CTC_REF_CLK_FREQ                (32768UL)       /* 32768Hz */

/* CTC TRMVAL value */
#define CTC_TRIM_VALUE                  (0x21U)         /* -31 */

/* Function clock gate definition  */
#define CTC_FCG_ENABLE()                (FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_CTC, ENABLE))

/* CTC interrupt definition */
#define CTC_ERR_IRQn                    (INT142_IRQn)

/* Baudrate definition  */
#define PRINTF_BAUDRATE                 (115200UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  BSP clock initialize.
 * @param  None
 * @retval None
 */
void BSP_CLK_Init(void)
{
    stc_clock_xtal_init_t stcXtalInit;

    /*********************** Configure output HRC clock ***********************/
    /* Configure clock output system clock */
    CLK_MCOConfig(CLK_MCO1, CLK_MCO_SRC_HRC, CLK_MCO_DIV1);

    /* Configure clock output pin */
    GPIO_SetFunc(MCO_PORT, MCO_PIN, MCO_GPIO_FUNC);

    /* Enable MCO output */
    CLK_MCOCmd(CLK_MCO1, ENABLE);

    /* HRC output */
    (void)CLK_HrcCmd(ENABLE);

    /************************** Configure XTAL32 clock ************************/
    /* Reset the VBAT area */
    PWC_VBAT_Reset();
    /* Initialize XTAL32 */
    (void)BSP_XTAL32_Init();

    /************************** Configure XTAL clock **************************/
    GPIO_AnalogCmd(BSP_XTAL_PORT, BSP_XTAL_PIN, ENABLE);
    (void)CLK_XtalStructInit(&stcXtalInit);
    stcXtalInit.u8State = CLK_XTAL_ON;
    (void)CLK_XtalInit(&stcXtalInit);

    /*********************** Switch system clock to XTAL **********************/
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_XTAL);
}

/**
 * @brief  CTC IRQ handler.
 * @param  None.
 * @retval None
 */
void CTC_IrqHandler(void)
{
    BSP_LED_On(LED_RED);
}

/**
 * @brief  CTC udf IRQ handler.
 * @param  None.
 * @retval None
 */
void CTC_Udf_IrqHandler(void)
{
    CTC_IrqHandler();
}

/**
 * @brief  CTC ovf IRQ handler.
 * @param  None.
 * @retval None
 */
void CTC_Ovf_IrqHandler(void)
{
    CTC_IrqHandler();
}

/**
 * @brief  Main function of CTC XTAL32 trimming project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint8_t u8InitTrimValue;
    uint8_t u8TrimmingValue;
    const stc_ctc_ct_init_t stcCtcInit = {
        .u32RefClockFreq = CTC_REF_CLK_FREQ,
        .u32RefClockSrc = CTC_REF_CLK_SRC,
        .u32RefClockDiv = CTC_REF_CLK_DIV8,
        .f32TolerantErrRate = 0.01F,
        .u8TrimValue = CTC_TRIM_VALUE,
    };

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize UART for debug print function. */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, PRINTF_BAUDRATE, BSP_PRINTF_Preinit);

    /* Initialize BSP expand IO. */
    BSP_IO_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Initialize BSP key. */
    BSP_KEY_Init();

    /* Enable peripheral clock */
    CTC_FCG_ENABLE();

    /* Wait CTC stop. */
    while (SET == CTC_GetStatus(CTC_FLAG_BUSY)) {
    }

    /* Initialize CTC function. */
    (void)CTC_CT_Init(&stcCtcInit);

    /* Register CTC error IRQ handler && configure NVIC. */
    (void)INTC_ShareIrqCmd(INT_SRC_CTC_ERR, ENABLE);
    NVIC_ClearPendingIRQ(CTC_ERR_IRQn);
    NVIC_SetPriority(CTC_ERR_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(CTC_ERR_IRQn);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    CTC_IntCmd(ENABLE);

    /* User key */
    while (SET != BSP_KEY_GetStatus(BSP_KEY_1)) {
    }

    u8InitTrimValue = CTC_GetTrimValue();

    CTC_Start();

    while (SET != CTC_GetStatus(CTC_FLAG_TRIM_OK)) {
    }

    u8TrimmingValue = CTC_GetTrimValue();

    CTC_Stop();

    /* Wait CTC stop. */
    while (SET == CTC_GetStatus(CTC_FLAG_BUSY)) {
    }

    DDL_Printf("Init trimming value is 0x%02X; Trimming result is 0x%02X. \r\n", u8InitTrimValue, u8TrimmingValue);

    BSP_LED_On(LED_BLUE);

    for (;;) {
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
