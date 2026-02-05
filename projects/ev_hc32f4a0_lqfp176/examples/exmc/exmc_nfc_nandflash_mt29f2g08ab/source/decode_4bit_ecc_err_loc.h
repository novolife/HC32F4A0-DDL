/**
 *******************************************************************************
 * @file  exmc/exmc_nfc_nandflash_mt29f2g08ab/source/decode_4bit_ecc_err_loc.h
 * @brief Head file for decoding ECC 4bit error location.
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
#ifndef __DECODE_4BIT_ECC_ERR_LOC_H__
#define __DECODE_4BIT_ECC_ERR_LOC_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>

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
int16_t NFC_SwDecodeEcc4BitsErrLocation(const int16_t ecc_syndrome[],
                                        int16_t ecc_err_byte_number[],
                                        int16_t ecc_err_byte_bit[],
                                        int16_t size);

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

#endif /* __DECODE_4BIT_ECC_ERR_LOC_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
