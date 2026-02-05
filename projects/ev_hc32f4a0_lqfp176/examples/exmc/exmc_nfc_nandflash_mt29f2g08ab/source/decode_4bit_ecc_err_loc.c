/**
 *******************************************************************************
 * @file  exmc/exmc_nfc_nandflash_mt29f2g08ab/source/decode_4bit_ecc_err_loc.c
 * @brief This file contains all the functions prototypes of the decoding error
 *        ECC 4bits location.
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
#include "hc32_ll_utility.h"

#include "a_to_i.h"
#include "i_to_a.h"
#include "decode_4bit_ecc_err_loc.h"

/**
 * @addtogroup HC32F4A0_DDL_Examples
 * @{
 */

/**
 * @addtogroup EXMC_NFC_Nandflash_MT29F2G08AB
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
 * @addtogroup EXMC_NFC_Nandflash_MT29F2G08AB_Global_Functions
 * @{
 */

/**
 * @brief  Software decode ECC 4bit error location for 512-bytes
 * @param  [in] ecc_syndrome            The syndrome value
 * @param  [out] ecc_err_byte_number    The ECC error byte number
 * @param  [out] ecc_err_byte_bit       The ECC error byte bit
 * @param  [in] size                    The array buffer size
 * @retval ECC error bytes count
 * @note the ecc_syndrome array size value must be 8.
 */
int16_t NFC_SwDecodeEcc4BitsErrLocation(const int16_t ecc_syndrome[],
                                        int16_t ecc_err_byte_number[],
                                        int16_t ecc_err_byte_bit[],
                                        int16_t size)
{
    int16_t i;
    int16_t j;
    int16_t elp_sum;
    int16_t Matrix_a[11] = {0};
    int16_t Matrix_b[11] = {0};
    int16_t Matrix_c[12] = {0};
    int16_t Element[7] = {0};
    int16_t alpha;
    int16_t temp_index;

    int16_t block_length = 8191;
    int16_t data_length = 4096;
    int16_t syn_val[20] = {0};
    int16_t syn_err = 0;
    int16_t err_count = 0;
    int16_t d_flg = 0;
    int16_t data_location;
    int16_t rev_location;
    int16_t result_byte;
    int16_t result_bit;
    int16_t idx;

    static int16_t err_location[20];
    static int16_t err_loc_ply[100][100];

    /* Initialize the whole array */
    for (i = 1; i <= 8; i++) {
        syn_val[i] = ecc_syndrome[i -  1];
    }

    for (i = 1; i <= 8; i++) {
        if (syn_val[i] != 0) {
            syn_err = 1;
        }
    }

    if (syn_err > 0) {
        /* initialise table entries */
        for (i = 1; i <= 8; i++) {
            syn_val[i] = i_to_a[syn_val[i]];
        }

        Matrix_c[0] = 0;
        Matrix_c[1] = syn_val[1];
        err_loc_ply[0][0] = 1;
        err_loc_ply[1][0] = 1;
        for (i = 1; i < 8; i++) {
            err_loc_ply[0][i] = 0;
            err_loc_ply[1][i] = 0;
        }
        Matrix_a[0] = 0;
        Matrix_a[1] = 0;
        Matrix_b[0] = -1;
        Matrix_b[1] = 0;
        alpha = -1;

        do {
            /* skip even loops */
            DDL_Printf("skip even loops\r\n");
            DDL_Printf("alpha = %8x\r\n", alpha);

            alpha += 2;
            if (Matrix_c[alpha] != -1) {
                temp_index = alpha - 2;

                if (temp_index < 0) {
                    temp_index = 0;
                }

                while ((Matrix_c[temp_index] == -1) && (temp_index > 0)) {
                    temp_index = temp_index - 2;
                }

                if (temp_index < 0) {
                    temp_index = 0;
                }

                if (temp_index > 0) {
                    j = temp_index;
                    do {
                        j = j - 2;

                        if (j < 0) {
                            j = 0;
                        }

                        if ((Matrix_c[j] != -1) && (Matrix_b[temp_index] < Matrix_b[j])) {
                            temp_index = j;
                        }
                    } while (j > 0);
                }

                if (Matrix_a[alpha] > Matrix_a[temp_index] + alpha - temp_index) {
                    Matrix_a[alpha + 2] = Matrix_a[alpha];
                } else {
                    Matrix_a[alpha + 2] = Matrix_a[temp_index] + alpha - temp_index;
                }

                for (i = 0; i < 8; i++) {
                    err_loc_ply[alpha + 2][i] = 0;
                }

                for (i = 0; i <= Matrix_a[temp_index]; i++) {
                    if (err_loc_ply[temp_index][i] != 0) {
                        idx = (Matrix_c[alpha] + block_length - Matrix_c[temp_index] + i_to_a[err_loc_ply[temp_index][i]]) % block_length;
                        if ((0 <= idx) && (idx < 8192)) {
                            err_loc_ply[alpha + 2][i + alpha - temp_index] = a_to_i[idx];
                        }
                    }
                }

                for (i = 0; i <= Matrix_a[alpha]; i++) {
                    err_loc_ply[alpha + 2][i] ^= err_loc_ply[alpha][i];
                }
            } else {
                Matrix_a[alpha + 2] = Matrix_a[alpha];
                for (i = 0; i <= Matrix_a[alpha]; i++) {
                    err_loc_ply[alpha + 2][i] = err_loc_ply[alpha][i];
                }
            }

            Matrix_b[alpha + 2] = alpha + 1 - Matrix_a[alpha + 2];

            /* Form (alpha+2)th discrepancy. */
            DDL_Printf("Form (alpha+2)th discrepancy\r\n");

            if (alpha < 8) {
                if (syn_val[alpha + 2] != -1) {
                    Matrix_c[alpha + 2] = a_to_i[syn_val[alpha + 2]];
                } else {
                    Matrix_c[alpha + 2] = 0;
                }

                for (i = 1; i <= Matrix_a[alpha + 2]; i++) {
                    if ((syn_val[alpha + 2 - i] != -1) && (err_loc_ply[alpha + 2][i] != 0)) {
                        idx = ((syn_val[alpha + 2 - i] + i_to_a[err_loc_ply[alpha + 2][i]]) % block_length);
                        if ((0 <= idx) && (idx < 8192)) {
                            Matrix_c[alpha + 2] ^= a_to_i[idx];
                        }
                    }
                }
                Matrix_c[alpha + 2] = i_to_a[Matrix_c[alpha + 2]];
            }
        } while ((alpha < 7) && (Matrix_a[alpha + 2] <= 4));

        alpha = alpha + 2;
        Matrix_a[7] = Matrix_a[alpha];
        if (Matrix_a[7] <= 4) {
            for (i = 1; i <= Matrix_a[7]; i++) {
                Element[i] = i_to_a[err_loc_ply[alpha][i]];
                DDL_Printf("iElement[%8x] = %8x\r\n", i, Element[i]);
            }

            err_count = 0 ;
            for (i = 1; i <= block_length; i++) {
                elp_sum = 1 ;
                for (j = 1; j <= Matrix_a[7]; j++) {
                    if (Element[j] != -1) {
                        Element[j] = (Element[j] + j) % block_length ;
                        elp_sum ^= a_to_i[Element[j]] ;
                        DDL_Printf("jElement[%8x] = %8x\r\n", j, Element[j]);
                        DDL_Printf("elp_sum = %8x\r\n", elp_sum);
                    }
                }

                if (0 == elp_sum) {
                    err_location[err_count] = block_length - i;
                    err_count++ ;
                    DDL_Printf("err_count = %8x\r\n", err_count);
                }
            }

            if (err_count == Matrix_a[7]) {
                d_flg = 1;
            }
        }
    } else {
        d_flg = 1;
        err_count = 0;
    }

    if (d_flg == 0) {
        DDL_Printf("Unable to decode !\r\n");
    } else {
        for (i = 0; i < err_count; i++) {
            DDL_Printf("error bit : %4d\r\n", i);
            if (err_location[i] >= 52) {
                data_location = err_location[i] - 52;
            } else {
                data_location = err_location[i] + data_length;
            }

            DDL_Printf("error location = %4d, ", data_location);

            if (data_location <= 4095) {
                rev_location = 4095 - data_location;
                result_bit = rev_location % 8;
                result_byte = (rev_location - result_bit) / 8;
                DDL_Printf("byte = %4d, bit = %4d\r\n", result_byte, result_bit);
            } else {
                rev_location = 4147 - data_location;
                result_bit = rev_location % 8;
                result_byte = (rev_location - result_bit) / 8;
                DDL_Printf("BCH byte = %4d, bit = %4d\r\n", result_byte, result_bit);
            }

            if (i < size) {
                ecc_err_byte_number[i] = result_byte;
                ecc_err_byte_bit[i] = result_bit;
            }
        }
    }

    return err_count;
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
