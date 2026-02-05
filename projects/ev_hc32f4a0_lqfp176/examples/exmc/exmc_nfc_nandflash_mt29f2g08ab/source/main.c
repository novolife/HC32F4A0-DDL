/**
 *******************************************************************************
 * @file  exmc/exmc_nfc_nandflash_mt29f2g08ab/source/main.c
 * @brief This example demonstrates NADN Flash function.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-10-31       CDT             Initialize buffer with random data
   2024-11-08       CDT             Fix bug: 4BitECC m_au8WriteDataHwEcc value error
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
 * @addtogroup EXMC_NFC_Nandflash_MT29F2G08AB
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup MT29F2G08AB_Operation_Timeout MT29F2G08AB Operation Timeout
 * @{
 */
#define MT29F2G08AB_ERASE_TIMEOUT           (2000000UL)
#define MT29F2G08AB_READ_TIMEOUT            (2000000UL)
#define MT29F2G08AB_READ_HWECC_TIMEOUT      (9000000UL)
#define MT29F2G08AB_WRITE_TIMEOUT           (2000000UL)
#define MT29F2G08AB_WRITE_HWECC_TIMEOUT     (2000000UL)
/**
 * @}
 */

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
 * @brief  Nandflash program test without ECC and spare
 * @param  [in] u32Page                 Program page
 * @retval int32_t:
 *           - LL_OK:                   No errors occurred.
 *           - LL_ERR:                  Errors occurred.
 */
static int32_t MT29F2G08AB_NoneEccAccess(uint32_t u32Page)
{
    uint32_t i;
    int32_t i32Ret = LL_ERR;

    __ALIGN_BEGIN static uint8_t m_au8ReadDataMeta[BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE];
    __ALIGN_BEGIN static uint8_t m_au8WriteDataMeta[BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE];

    /* Initialize data. */
    for (i = 0U; i < BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE; i++) {
        m_au8WriteDataMeta[i] = (uint8_t)rand();
    }

    /* Write page: 2048Bytes */
    if (LL_OK == BSP_MT29F2G08AB_WritePage(u32Page, m_au8WriteDataMeta, \
                                           BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE, MT29F2G08AB_WRITE_TIMEOUT)) {
        /* Read page: 2048Bytes */
        if (LL_OK == BSP_MT29F2G08AB_ReadPage(u32Page, m_au8ReadDataMeta, \
                                              BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE, MT29F2G08AB_READ_TIMEOUT)) {
            if (0 == memcmp(m_au8WriteDataMeta, m_au8ReadDataMeta, BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE)) {
                i32Ret = LL_OK;
            }
        }
    }

    return i32Ret;
}

/**
 * @brief  Nandflash program test without ECC
 * @param  [in] u32Page                 Program page
 * @retval int32_t:
 *           - LL_OK:                   No errors occurred.
 *           - LL_ERR:                  Errors occurred.
 */
static int32_t MT29F2G08AB_NoneEccAccessSpare(uint32_t u32Page)
{
    uint32_t i;
    int32_t i32Ret = LL_ERR;

    __ALIGN_BEGIN static uint8_t m_au8ReadDataMetaWithSpare[BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE];
    __ALIGN_BEGIN static uint8_t m_au8WriteDataMetaWithSpare[BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE];

    /* Initialize data. */
    for (i = 0U; i < BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE; i++) {
        m_au8WriteDataMetaWithSpare[i] = (uint8_t)rand();
    }

    /* Write page: 2048 + 64Bytes */
    if (LL_OK == BSP_MT29F2G08AB_WritePage(u32Page, m_au8WriteDataMetaWithSpare, \
                                           BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE, MT29F2G08AB_WRITE_TIMEOUT)) {
        /* Read page: 2048 + 64Bytes */
        if (LL_OK == BSP_MT29F2G08AB_ReadPage(u32Page, m_au8ReadDataMetaWithSpare, \
                                              BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE, MT29F2G08AB_READ_TIMEOUT)) {
            if (0 == memcmp(m_au8WriteDataMetaWithSpare, m_au8ReadDataMetaWithSpare, BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE)) {
                i32Ret = LL_OK;
            }
        }
    }

    return i32Ret;
}

/**
 * @brief  Nandflash program test with hardware ECC 1 bit
 * @param  [in] u32Page                 Program page
 * @retval int32_t:
 *           - LL_OK:                   No errors occurred.
 *           - LL_ERR:                  Errors occurred.
 */
static int32_t MT29F2G08AB_1BitEccAccess(uint32_t u32Page)
{
    uint32_t i;
    int32_t i32Ret = LL_ERR;

    __ALIGN_BEGIN static uint8_t m_au8ReadDataHwEcc[BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE];
    __ALIGN_BEGIN static uint8_t m_au8WriteDataHwEcc[BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE];
    __ALIGN_BEGIN static uint8_t m_au8SwEcc1Bit[BSP_MT29F2G08AB_PAGE_1BIT_ECC_VALUE_SIZE];

    /* Initialize data. */
    for (i = 0UL; i < BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE; i++) {
        m_au8WriteDataHwEcc[i] = (uint8_t)rand();
    }
    m_au8WriteDataHwEcc[1] = 1U;

    /* Enable ECC 1bit: write 2048Bytes */
    if (LL_OK == BSP_MT29F2G08AB_1BitEccWritePage(u32Page, m_au8WriteDataHwEcc, \
                                                  BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE, MT29F2G08AB_WRITE_HWECC_TIMEOUT)) {
        /* Disable ECC 1bit: read 2048 + 64Bytes */
        (void)BSP_MT29F2G08AB_ReadPage(u32Page, m_au8ReadDataHwEcc, \
                                       BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE, MT29F2G08AB_READ_TIMEOUT);

        /* Software calculate ECC 1bit */
        (void)NFC_SwCalculateEcc1Bit(&m_au8WriteDataHwEcc[0], &m_au8SwEcc1Bit[0]);
        (void)NFC_SwCalculateEcc1Bit(&m_au8WriteDataHwEcc[512], &m_au8SwEcc1Bit[3]);
        (void)NFC_SwCalculateEcc1Bit(&m_au8WriteDataHwEcc[1024], &m_au8SwEcc1Bit[6]);
        (void)NFC_SwCalculateEcc1Bit(&m_au8WriteDataHwEcc[1536], &m_au8SwEcc1Bit[9]);

        /* Compare software & hardware calculating ECC result */
        if (0 == memcmp(m_au8SwEcc1Bit, \
                        &m_au8ReadDataHwEcc[BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE - BSP_MT29F2G08AB_PAGE_1BIT_ECC_VALUE_SIZE], \
                        BSP_MT29F2G08AB_PAGE_1BIT_ECC_VALUE_SIZE)) {
            /* Modify the 2nd byte value from 0x01 to 0x00 */
            m_au8ReadDataHwEcc[1] = 0x00U;

            /* Disable ECC 1bit: write 2048 + 64Bytes */
            (void)BSP_MT29F2G08AB_WritePage(u32Page, m_au8ReadDataHwEcc, \
                                            BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE, MT29F2G08AB_WRITE_TIMEOUT);

            /* Enable ECC 1bit: read 2048Bytes */
            (void)BSP_MT29F2G08AB_1BitEccReadPage(u32Page, m_au8ReadDataHwEcc, \
                                                  BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE, MT29F2G08AB_READ_HWECC_TIMEOUT);

            /* Single bit error: the 2nd byte - bit 0 */
            if (EXMC_NFC_Get1BitEccResult(EXMC_NFC_ECC_SECTION0) == EXMC_NFC_1BIT_ECC_SINGLE_BIT_ERR) {
                if ((EXMC_NFC_Get1BitEccErrBitLocation(EXMC_NFC_ECC_SECTION0) == EXMC_NFC_1BIT_ECC_ERR_BIT0) && \
                    (EXMC_NFC_Get1BitEccErrByteLocation(EXMC_NFC_ECC_SECTION0) == 1UL)) {
                    i32Ret = LL_OK; /* The result meets the expected */
                }
            }
        }
    }

    return i32Ret;
}

/**
 * @brief  Nandflash program test with hardware ECC 4bits
 * @param  [in] u32Page                 Program page
 * @retval int32_t:
 *           - LL_OK:                   No errors occurred.
 *           - LL_ERR:                  Errors occurred.
 */
static int32_t MT29F2G08AB_4BitEccAccess(uint32_t u32Page)
{
    uint32_t i;
    int32_t i32Ret = LL_ERR;
    uint16_t au16SynVal[8];
    int16_t ai16EccTestErrByteNumber[4];
    int16_t ai16EccTestErrByteBit[4];
    int16_t ai16EccExpectedErrByteNumber[4];
    int16_t ai16EccExpectedErrByteBit[4];

    __ALIGN_BEGIN static uint8_t m_au8ReadDataHwEcc[BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE];
    __ALIGN_BEGIN static uint8_t m_au8WriteDataHwEcc[BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE];

    /* Initialize data. */
    for (i = 0UL; i < BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE; i++) {
        m_au8WriteDataHwEcc[i] = (uint8_t)rand();
    }
    m_au8WriteDataHwEcc[254] = 0xFEU;
    m_au8WriteDataHwEcc[255] = 0xFFU;

    /* Enable ECC 4bit: write 2048Bytes */
    if (LL_OK == BSP_MT29F2G08AB_4BitEccWritePage(u32Page, m_au8WriteDataHwEcc, \
                                                  BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE, MT29F2G08AB_WRITE_HWECC_TIMEOUT)) {
        /* Enable ECC 4bit: read 2048Bytes */
        i32Ret = BSP_MT29F2G08AB_4BitEccReadPage(u32Page, m_au8ReadDataHwEcc, \
                                                 BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE, MT29F2G08AB_READ_HWECC_TIMEOUT);

        if (LL_OK == i32Ret) {
            /* Check whether ECC errors occur */
            if (EXMC_NFC_GetStatus(EXMC_NFC_FLAG_ECC_ERR) == RESET) {
                /* Disable ECC 4bit: read 2048 + 64Bytes */
                (void)BSP_MT29F2G08AB_ReadPage(u32Page, m_au8ReadDataHwEcc, \
                                               BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE, MT29F2G08AB_READ_TIMEOUT);

                /* Modify data to create error */
                m_au8ReadDataHwEcc[254] = 0xAEU;  /* Modify the 254th byte value from 0xFE to 0xAE */
                m_au8ReadDataHwEcc[255] = 0xF5U;  /* Modify the 255th byte value from 0xFF to 0xF5 */

                /* Error bit: the 254th byte - 2 bit */
                ai16EccExpectedErrByteNumber[0] = 254;
                ai16EccExpectedErrByteBit[0] = 4;

                /* Error bit: the 254th byte - 3 bit */
                ai16EccExpectedErrByteNumber[1] = 254;
                ai16EccExpectedErrByteBit[1] = 6;

                /* Error bit: the 255th byte - 1 bit */
                ai16EccExpectedErrByteNumber[2] = 255;
                ai16EccExpectedErrByteBit[2] = 1;

                /* Error bit: the 255th byte - 3 bit */
                ai16EccExpectedErrByteNumber[3] = 255;
                ai16EccExpectedErrByteBit[3] = 3;

                /* Disable ECC 4bit: write 2048 + 64Bytes */
                (void)BSP_MT29F2G08AB_WritePage(u32Page, m_au8ReadDataHwEcc, \
                                                BSP_MT29F2G08AB_PAGE_SIZE_WITH_SPARE, MT29F2G08AB_WRITE_TIMEOUT);

                /* Enable ECC 4bit: read 2048Bytes */
                (void)BSP_MT29F2G08AB_4BitEccReadPage(u32Page, m_au8ReadDataHwEcc, \
                                                      BSP_MT29F2G08AB_PAGE_SIZE_WITHOUT_SPARE, MT29F2G08AB_READ_HWECC_TIMEOUT);
                /* Get ECC syndrome */
                (void)EXMC_NFC_GetSyndrome(EXMC_NFC_ECC_SECTION0, au16SynVal, 8U);

                /* Decode ECC errors location */
                i32Ret = LL_ERR;
                if (4 == NFC_SwDecodeEcc4BitsErrLocation((const int16_t *)((uint32_t)(&au16SynVal)), \
                                                         ai16EccTestErrByteNumber, ai16EccTestErrByteBit, 4)) {
                    if ((ai16EccExpectedErrByteBit[0] == ai16EccTestErrByteBit[0]) && \
                        (ai16EccExpectedErrByteBit[1] == ai16EccTestErrByteBit[1]) && \
                        (ai16EccExpectedErrByteBit[2] == ai16EccTestErrByteBit[2]) && \
                        (ai16EccExpectedErrByteBit[3] == ai16EccTestErrByteBit[3]) && \
                        (ai16EccExpectedErrByteNumber[0] == ai16EccTestErrByteNumber[0]) && \
                        (ai16EccExpectedErrByteNumber[1] == ai16EccTestErrByteNumber[1]) && \
                        (ai16EccExpectedErrByteNumber[2] == ai16EccTestErrByteNumber[2]) && \
                        (ai16EccExpectedErrByteNumber[3] == ai16EccTestErrByteNumber[3])) {
                        i32Ret = LL_OK; /* The result meets the expected */
                    }
                }
            }
        }
    }

    return i32Ret;
}

/**
 * @brief  Main function of EXMC_NFC NAND Flash project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint8_t au8DevId[4];
    uint8_t u8ErrCount = 0U;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* Initialize system clock: */
    BSP_CLK_Init();

    /* EXCLK 60MHz */
    CLK_SetClockDiv(CLK_BUS_EXCLK, CLK_EXCLK_DIV4);

    /* Initialize LED */
    BSP_IO_Init();
    BSP_LED_Init();

    /* Configure nandflash */
    (void)BSP_MT29F2G08AB_Init();

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* Read ID */
    (void)BSP_MT29F2G08AB_ReadId(0UL, au8DevId, sizeof(au8DevId), MT29F2G08AB_READ_TIMEOUT);
    if ((au8DevId[0] == BSP_MT29F2G08ABAEA_MANUFACTURER_ID) || \
        (au8DevId[1] == BSP_MT29F2G08ABAEA_DEVICE_ID1) || \
        (au8DevId[2] == BSP_MT29F2G08ABAEA_DEVICE_ID2) || \
        (au8DevId[3] == BSP_MT29F2G08ABAEA_DEVICE_ID3)) {
        /* Erase nandflash. */
        if (LL_OK == BSP_MT29F2G08AB_EraseBlock(0UL, MT29F2G08AB_ERASE_TIMEOUT)) {
            if (LL_OK != MT29F2G08AB_NoneEccAccess(0UL)) {
                u8ErrCount++;
            }

            if (LL_OK != MT29F2G08AB_NoneEccAccessSpare(1UL)) {
                u8ErrCount++;
            }

            if (LL_OK != MT29F2G08AB_1BitEccAccess(2UL)) {
                u8ErrCount++;
            }

            if (LL_OK != MT29F2G08AB_4BitEccAccess(3UL)) {
                u8ErrCount++;
            }
        } else {
            u8ErrCount++;
        }
    } else {
        u8ErrCount++;
    }

    if (u8ErrCount > 0U) {
        BSP_LED_On(LED_RED);
    } else {
        BSP_LED_On(LED_BLUE);
    }

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
