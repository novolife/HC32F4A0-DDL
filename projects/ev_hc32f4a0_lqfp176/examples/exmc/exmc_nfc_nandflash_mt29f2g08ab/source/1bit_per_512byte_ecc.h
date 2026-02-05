/**
 *******************************************************************************
 * @file  exmc/exmc_nfc_nandflash_mt29f2g08ab/source/1bit_per_512byte_ecc.h
 * @brief  * @brief Head file for calculating 1bit ECC.
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
#ifndef __1BIT_PER_512BYTES_ECC_H__
#define __1BIT_PER_512BYTES_ECC_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll_def.h"

/**
 * @addtogroup HC32F4A0_DDL_Examples
 * @{
 */

/**
 * @addtogroup EXMC_NFC_Nandflash_MT29F2G08AB
 * @{
 */

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
/**
 * @addtogroup EXMC_NFC_Nandflash_MT29F2G08AB_Global_Functions
 * @{
 */
int32_t NFC_SwCalculateEcc1Bit(const uint8_t au8Data[], uint8_t au8Ecc[]);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif /* __1BIT_PER_512BYTES_ECC_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
