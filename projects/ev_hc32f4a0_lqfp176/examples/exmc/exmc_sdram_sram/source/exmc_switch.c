/**
 *******************************************************************************
 * @file  exmc/exmc_sdram_sram/source/exmc_switch.c
 * @brief This example demonstrates SDRAM and SRAM function.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-10-31       CDT             First version
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
#include "exmc_switch.h"

/**
 * @addtogroup EXMC_SDRAM_SRAM
 * @{
 */

/**
 * @addtogroup EXMC_Switch EXMC Switch
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
/**
 * @defgroup EXMC_Switch_Global_Variables EXMC_Switch Global Variables
 * @{
 */
__IO uint16_t g_u16ReadDummy = 0UL;
uint32_t g_u32ReadDummyDmcAddr = 0UL;
uint32_t g_u32ReadDummySmcAddr = 0UL;
/**
 * @}
 */

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
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
