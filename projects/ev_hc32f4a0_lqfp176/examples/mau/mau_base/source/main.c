/**
 *******************************************************************************
 * @file  mau/mau_base/source/main.c
 * @brief Main program of MAU base for the Device Driver Library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2024-11-08       CDT             Optimize software calculate sqrt formula
                                    Use MRC and set PCLK4 for better random data
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
#include "math.h"
#include "main.h"

/**
 * @addtogroup HC32F4A0_DDL_Examples
 * @{
 */

/**
 * @addtogroup MAU
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define MAU_SQRT_IRQn                   (INT008_IRQn)
#define MAU_SQRT_INTSRC                 (INT_SRC_MAU_SQRT)

#define PI                              (3.1415926F)
#define SIN_DOT_CNT                     (4096U)
#define SIN_DELTA                       (1U)
#define SQRT_FIXED_BIT                  (8U)
#define SQRT_DATA_CNT                   (4096U)
#define SQRT_DELTA                      (1UL << (SQRT_FIXED_BIT / 2U))

#define LED_DLY_MS                      (200U)
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
 * @brief   MAU Initialization
 * @param   None
 * @retval  None
 */
static void MAU_Init(void)
{
    /* Enable MAU peripheral clock. */
    FCG_Fcg0PeriphClockCmd(PWC_FCG0_MAU, ENABLE);
}

/**
 * @brief  Check whether the delta between two values is below threshold
 * @param  [in] u32VarA       First input variable
 * @param  [in] u32VarB       Second input variable
 * @param  [in] u32MaxError   threshold
 * @retval Status
 *           0: Delta is below threshold
 *           1: Delta is above threshold
 */
static uint8_t MAU_CheckResidualErr(uint32_t u32VarA, uint32_t u32VarB, uint32_t u32MaxError)
{
    uint8_t status = 0U;
    if (u32VarA >= u32VarB) {
        if ((u32VarA - u32VarB) > u32MaxError) {
            status = 1U;
        }
    } else {
        if ((u32VarB - u32VarA) > u32MaxError) {
            status = 1U;
        }
    }
    return status;
}

/**
 * @brief  Error Handler, toggle led red if u32IsError is 1, else turn led blue on
 * @param  [in] u32IsError
 * @retval  None
 */
static void ErrHandler(uint32_t u32IsError)
{
    if (u32IsError != 0U) {
        for (;;) {
            BSP_LED_Toggle(LED_RED);
            DDL_DelayMS(LED_DLY_MS);
        }
    } else {
        BSP_LED_On(LED_BLUE);
        for (;;) {
            /* do nothing */
        }
    }
}

/**
 * @brief   Interrupt callback function of sqrt Done
 * @param   None
 * @retval  None
 */
static void MAU_SqrtIrqCallback(void)
{
    (void)MAU_SqrtReadData(CM_MAU);
}

/**
 * @brief   Sqrt interrupt init
 * @param   None
 * @retval  None
 */
static void Sqrt_IntCfg(void)
{
    stc_irq_signin_config_t stcIrqSignCfg;
    MAU_SqrtIntCmd(CM_MAU, ENABLE);

    /* Register IRQ handler && configure NVIC. */
    stcIrqSignCfg.enIRQn = MAU_SQRT_IRQn;
    stcIrqSignCfg.enIntSrc = MAU_SQRT_INTSRC;
    stcIrqSignCfg.pfnCallback = &MAU_SqrtIrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignCfg);
    NVIC_ClearPendingIRQ(stcIrqSignCfg.enIRQn);
    NVIC_SetPriority(stcIrqSignCfg.enIRQn, DDL_IRQ_PRIO_03);
    NVIC_EnableIRQ(stcIrqSignCfg.enIRQn);
}

/**
 * @brief  Generate random data as radicand parameters for sqrt example
 * @param  [in] u32TrngArr  array to hold random data
 * @param  [in] count       number of random data that will be generated
 * @retval None
 */
static void  Trng_Gen(uint32_t u32TrngArr[], uint32_t count)
{
    uint32_t i, trngloop = count / 2U;

    /* Enable TRNG. */
    FCG_Fcg0PeriphClockCmd(PWC_FCG0_TRNG, ENABLE);
    /* Shift 64 times, Disable the Load function */
    TRNG_Init(TRNG_SHIFT_CNT64, TRNG_RELOAD_INIT_VAL_ENABLE);
    TRNG_Cmd(ENABLE);
    for (i = 0U; i < trngloop; i++) {
        (void)TRNG_GenerateRandom(&u32TrngArr[i * 2U], 2);
    }
}

/**
 * @brief Compare the sqrt result generated by mau with the result generated by C library math.h
 * @param  None
 * @retval u32Err
 *           0: Delta is below threshold
 *           1: Delta is above threshold
 */
static uint32_t Sqrt_Example(void)
{
    static uint32_t u32Radicands[SQRT_DATA_CNT];
    static uint32_t u32SplGrp[SQRT_DATA_CNT];
    static uint32_t u32CtrlGrp[SQRT_DATA_CNT];
    int32_t i32Ret = LL_OK;

    /* Generate random data as radicand parameters for sqrt example */
    Trng_Gen(u32Radicands, SQRT_DATA_CNT);
    /* Sqrt configuration */
    Sqrt_IntCfg();
    /* For prevent result overflow, make left shift by software. */
    MAU_SqrtResultLShiftConfig(CM_MAU, 0U);
    /* Input random radicands, generate sqrt and compare results */
    for (uint32_t i = 0U; i < SQRT_DATA_CNT; i++) {
        /* Generate sample group by mau */
        i32Ret = MAU_Sqrt(CM_MAU, u32Radicands[i], &u32SplGrp[i]);
        if (LL_OK == i32Ret) {
            /* left shift by software. */
            u32SplGrp[i] = u32SplGrp[i] << (SQRT_FIXED_BIT / 2U);
            /* Generate control group by math.h */
            u32CtrlGrp[i] = (uint32_t)((sqrt((((double)u32Radicands[i]) / (double)(1UL << SQRT_FIXED_BIT)))) * (1UL << SQRT_FIXED_BIT));
            /* Compare result to check error*/
            if (MAU_CheckResidualErr(u32SplGrp[i], (u32CtrlGrp[i]), SQRT_DELTA) == 1U) {
                i32Ret = LL_ERR;
                break;
            }
        }
    }
    return (uint32_t)i32Ret;
}

/**
 * @brief  Compare the sine result generated by mau with the result generated by C library math.h
 * @param  None
 * @retval u32Err
 *           0: Delta is below threshold
 *           1: Delta is above threshold
 */
static uint32_t Sin_Example(void)
{
    static uint32_t u32SplGrp[SIN_DOT_CNT];
    static uint32_t u32CtrlGrp[SIN_DOT_CNT];
    volatile uint16_t u16AvgAngIdx = (uint16_t)(float32_t)((float32_t)MAU_SIN_ANGIDX_TOTAL / (float32_t)SIN_DOT_CNT + (float32_t)0.5);
    uint32_t u32Err = 0U;
    float32_t fAvgRadian;

    if (u16AvgAngIdx == 0U) {
        u16AvgAngIdx = 1U;
    }
    fAvgRadian = (float32_t)u16AvgAngIdx / (float32_t)MAU_SIN_ANGIDX_TOTAL * (float32_t)2 * PI;
    for (uint32_t i = 0U; i < SIN_DOT_CNT; i++) {
        /* Generate sqrt sample group by mau */
        u32SplGrp[i] = (uint32_t)MAU_Sin(CM_MAU, (uint16_t)(u16AvgAngIdx * i));
        /* Generate sqrt control group by math.h */
        u32CtrlGrp[i] = (uint32_t)(int32_t)(float32_t)(sin(fAvgRadian * (float64_t)i) * (float32_t)MAU_SIN_Q15_SCALAR);
        /* Compare result to check error*/
        if (MAU_CheckResidualErr(u32SplGrp[i], (u32CtrlGrp[i]), SIN_DELTA) == 1U) {
            u32Err = 1U;
            break;
        }
    }
    return u32Err;
}

/**
 * @brief  Main function of MAU project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                 LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* Use MRC for system clock, and set PCLK4 for better random data. */
    CLK_SetClockDiv(CLK_BUS_PCLK4, CLK_SYSCLK_DIV8);
    BSP_IO_Init();
    BSP_LED_Init();

    uint32_t u32ErrSqrt, u32ErrSin;
    /* 0.MAU initialization*/
    MAU_Init();

    /* 1.Compare the sqrt result generated by mau with the result generated by C library math.h*/
    u32ErrSqrt = Sqrt_Example();
    /* 2.Compare the sine result generated by mau with the result generated by C library math.h*/
    u32ErrSin = Sin_Example();

    /* 3.Error handler, turn led blue on if u32ErrSqrt and u32ErrSin are both 0, else toggle led red*/
    ErrHandler((u32ErrSqrt | u32ErrSin));
    return 0;
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
