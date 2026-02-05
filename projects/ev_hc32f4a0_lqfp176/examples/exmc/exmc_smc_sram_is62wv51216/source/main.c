/**
 *******************************************************************************
 * @file  exmc/exmc_smc_sram_is62wv51216/source/main.c
 * @brief This example demonstrates SMC SRAM function.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-10-31       CDT             Initialize buffer with random data
   2023-09-30       CDT             Modify EXCLK frequency: 60MHz -> 30MHz
                                    Fix typos and modify file brief
                                    Fix memory address printf value
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
 * @addtogroup EXMC_SMC_SRAM_IS62WV51216
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define DATA_BUF_LEN                    (0x8000UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static uint32_t m_u32StartAddr = 0UL;
static uint32_t m_u32ByteSize = 0UL;

static uint8_t m_au8ReadData[DATA_BUF_LEN];
static uint8_t m_au8WriteData[DATA_BUF_LEN];

static uint16_t m_au16ReadData[DATA_BUF_LEN];
static uint16_t m_au16WriteData[DATA_BUF_LEN];

static uint32_t m_au32ReadData[DATA_BUF_LEN];
static uint32_t m_au32WriteData[DATA_BUF_LEN];

static uint32_t m_u32TestCnt = 0UL;
static uint32_t m_u32ByteTestErrorCnt = 0UL;
static uint32_t m_u32HalfwordTestErrorCnt = 0UL;
static uint32_t m_u32WordTestErrorCnt = 0UL;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Initialize test data.
 * @param  None
 * @retval None
 */
static void InitTestData(void)
{
    uint32_t i;

    /* Clear count value */
    m_u32ByteTestErrorCnt = 0UL;
    m_u32HalfwordTestErrorCnt = 0UL;
    m_u32WordTestErrorCnt = 0UL;

    /* Initialize test data. */
    for (i = 0UL; i < DATA_BUF_LEN; i++) {
        m_au8ReadData[i] = 0U;
        m_au16ReadData[i] = 0U;
        m_au32ReadData[i] = 0UL;
        m_au8WriteData[i] = (uint8_t)rand();
        m_au16WriteData[i] = (uint16_t)rand();
        m_au32WriteData[i] = (uint32_t)rand();
    }
}

/**
 * @brief  Write memory for byte.
 * @param  [in] u32Addr                 Memory address to write
 * @param  [in] au8Data                 Pointer to source buffer
 * @param  [in] u32Len                  Length of the buffer to write to memory
 * @retval int32_t:
 *           - LL_OK:                   Write successfully.
 *           - LL_ERR_INVD_PARAM:       If one of following cases matches:
 *                                      -The pointer au8Data value is NULL.
 *                                      -The variable u32Len value is 0.
 */
static int32_t MEMORY_Write8(uint32_t u32Addr, const uint8_t au8Data[], uint32_t u32Len)
{
    uint32_t i;
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if ((au8Data != NULL) && (0UL != u32Len)) {
        for (i = 0UL; i < u32Len; i++) {
            *(uint8_t *)(u32Addr + i) = au8Data[i];
        }
        i32Ret = LL_OK;
    }

    return i32Ret;
}

/**
 * @brief  Read memory for byte.
 * @param  [in] u32Addr                 Memory address to read
 * @param  [out] au8Data                Pointer to buffer
 * @param  [in] u32Len                  Length of the buffer to read from memory
 * @retval int32_t:
 *           - LL_OK:                   Read successfully.
 *           - LL_ERR_INVD_PARAM:       If one of following cases matches:
 *                                      -The pointer au8Data value is NULL.
 *                                      -The variable u32Len value is 0.
 */
static int32_t MEMORY_Read8(uint32_t u32Addr, uint8_t au8Data[], uint32_t u32Len)
{
    uint32_t i;
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if ((NULL != au8Data) && (0UL != u32Len)) {
        for (i = 0UL; i < u32Len; i++) {
            au8Data[i] = *(uint8_t *)(u32Addr + i);
        }
        i32Ret = LL_OK;
    }

    return i32Ret;
}

/**
 * @brief  Write memory for half-word.
 * @param  [in] u32Addr                 Memory address to write
 * @param  [in] au16Data                Pointer to buffer
 * @param  [in] u32Len                  Length of the buffer to write to memory
 * @retval int32_t:
 *           - LL_OK:                   Write successfully.
 *           - LL_ERR_INVD_PARAM:       If one of following cases matches:
 *                                      -The pointer au16Data value is NULL.
 *                                      -The variable u32Len value is 0.
 */
static int32_t MEMORY_Write16(uint32_t u32Addr, const uint16_t au16Data[], uint32_t u32Len)
{
    uint32_t i;
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if ((au16Data != NULL) && (0UL != u32Len)) {
        for (i = 0UL; i < u32Len; i++) {
            *((uint16_t *)u32Addr) = au16Data[i];
            u32Addr += 2UL;
        }
        i32Ret = LL_OK;
    }

    return i32Ret;
}

/**
 * @brief  Read memory for half-word.
 * @param  [in] u32Addr                 Memory address to read
 * @param  [out] au16Data               Pointer to buffer
 * @param  [in] u32Len                  Length of the buffer to read from memory
 * @retval int32_t:
 *           - LL_OK:                   Read successfully.
 *           - LL_ERR_INVD_PARAM:       If one of following cases matches:
 *                                      -The pointer au16Data value is NULL.
 *                                      -The variable u32Len value is 0.
 */
static int32_t MEMORY_Read16(uint32_t u32Addr, uint16_t au16Data[], uint32_t u32Len)
{
    uint32_t i;
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if ((NULL != au16Data) && (0UL != u32Len)) {
        for (i = 0UL; i < u32Len; i++) {
            au16Data[i] = *((uint16_t *)u32Addr);
            u32Addr += 2UL;
        }
        i32Ret = LL_OK;
    }

    return i32Ret;
}

/**
 * @brief  Write memory for word.
 * @param  [in] u32Addr                 Memory address to write
 * @param  [in] au32Data                Pointer to source buffer
 * @param  [in] u32Len                  Length of the buffer to write to memory
 * @retval int32_t:
 *           - LL_OK:                   Read successfully.
 *           - LL_ERR_INVD_PARAM:       If one of following cases matches:
 *                                      -The pointer au32Data value is NULL.
 *                                      -The variable u32Len value is 0.
 */
static int32_t MEMORY_Write32(uint32_t u32Addr, const uint32_t au32Data[], uint32_t u32Len)
{
    uint32_t i;
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if ((NULL != au32Data) && (0UL != u32Len)) {
        for (i = 0UL; i < u32Len; i++) {
            *((uint32_t *)u32Addr) = au32Data[i];
            u32Addr += 4UL;
        }
        i32Ret = LL_OK;
    }

    return i32Ret;
}

/**
 * @brief  Read memory for word.
 * @param  [in] u32Addr                 Memory address to read
 * @param  [out] au32Data               Pointer to destination buffer to write
 * @param  [in] u32Len                  Length of the buffer to read from memory
 * @retval int32_t:
 *           - LL_OK:                   Read successfully.
 *           - LL_ERR_INVD_PARAM:       If one of following cases matches:
 *                                      -The pointer au32Data value is NULL.
 *                                      -The variable u32Len value is 0.
 */
static int32_t MEMORY_Read32(uint32_t u32Addr, uint32_t au32Data[], uint32_t u32Len)
{
    uint32_t i;
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if ((NULL != au32Data) && (0UL != u32Len)) {
        for (i = 0UL; i < u32Len; i++) {
            au32Data[i] = *((uint32_t *)u32Addr);
            u32Addr += 4UL;
        }
        i32Ret = LL_OK;
    }

    return i32Ret;
}

/**
 * @brief  Access memory for byte.
 * @param  [in] u3AccessAddr:           Memory address
 * @param  [in] NumBytes:               Access size(unit: byte)
 * @retval count for reading and writing data error
 */
static uint32_t MEMORY_Access8(uint32_t u3AccessAddr, uint32_t NumBytes)
{
    uint32_t i;
    uint32_t j;
    uint32_t u32TestErrCnt = 0UL;
    uint32_t u32MemoryAddr = u3AccessAddr;

    for (i = 0UL; i < NumBytes; i += DATA_BUF_LEN) {
        (void)MEMORY_Write8(u32MemoryAddr, m_au8WriteData, DATA_BUF_LEN);
        (void)MEMORY_Read8(u32MemoryAddr, m_au8ReadData, DATA_BUF_LEN);

        /* Verify data. */
        for (j = 0UL; j < DATA_BUF_LEN; j++) {
            if (m_au8WriteData[j] != m_au8ReadData[j]) {
                u32TestErrCnt++;
                DDL_Printf("Byte read/write error: address = 0x%.8x; write data = 0x%x; read data = 0x%x\r\n",
                           (unsigned int)(u32MemoryAddr + j * sizeof(m_au8ReadData[0])), (unsigned int)m_au8WriteData[j], (unsigned int)m_au8ReadData[j]);
            }
        }

        u32MemoryAddr += (DATA_BUF_LEN * sizeof(m_au8ReadData[0]));
        (void)memset(m_au8ReadData, 0, (DATA_BUF_LEN * sizeof(m_au8ReadData[0])));
        BSP_LED_Toggle(LED_BLUE);
    }

    return u32TestErrCnt;
}

/**
 * @brief  Access memory for half-word.
 * @param  [in] u3AccessAddr:           Memory address
 * @param  [in] NumHalfwords:           Access size(unit: half-word)
 * @retval count for reading and writing data error
 */
static uint32_t MEMORY_Access16(uint32_t u3AccessAddr, uint32_t NumHalfwords)
{
    uint32_t i;
    uint32_t j;
    uint32_t u32TestErrCnt = 0UL;
    uint32_t u32MemoryAddr = u3AccessAddr;

    for (i = 0UL; i < NumHalfwords; i += DATA_BUF_LEN) {
        (void)MEMORY_Write16(u32MemoryAddr, m_au16WriteData, DATA_BUF_LEN);
        (void)MEMORY_Read16(u32MemoryAddr, m_au16ReadData, DATA_BUF_LEN);

        /* Verify data. */
        for (j = 0UL; j < DATA_BUF_LEN; j++) {
            if (m_au16WriteData[j] != m_au16ReadData[j]) {
                u32TestErrCnt++;
                DDL_Printf("Halfword read/write error: address = 0x%.8x; write data = 0x%x; read data = 0x%x\r\n",
                           (unsigned int)(u32MemoryAddr + j * sizeof(m_au16ReadData[0])), (unsigned int)m_au16WriteData[j], (unsigned int)m_au16ReadData[j]);
            }
        }

        u32MemoryAddr += (DATA_BUF_LEN * sizeof(m_au16ReadData[0]));
        (void)memset(m_au16ReadData, 0, (DATA_BUF_LEN * sizeof(m_au16ReadData[0])));
        BSP_LED_Toggle(LED_BLUE);
    }

    return u32TestErrCnt;
}

/**
 * @brief  Access memory for word.
 * @param  [in] u3AccessAddr:           Memory address
 * @param  [in] NumWords:               Access size(unit: word)
 * @retval count for reading and writing data error
 */
static uint32_t MEMORY_Access32(uint32_t u3AccessAddr, uint32_t NumWords)
{
    uint32_t i;
    uint32_t j;
    uint32_t u32TestErrCnt = 0UL;
    uint32_t u32MemoryAddr = u3AccessAddr;

    for (i = 0UL; i < NumWords; i += DATA_BUF_LEN) {
        (void)MEMORY_Write32(u32MemoryAddr, m_au32WriteData, DATA_BUF_LEN);
        (void)MEMORY_Read32(u32MemoryAddr, m_au32ReadData, DATA_BUF_LEN);

        /* Verify data. */
        for (j = 0UL; j < DATA_BUF_LEN; j++) {
            if (m_au32WriteData[j] != m_au32ReadData[j]) {
                u32TestErrCnt++;
                DDL_Printf("Word read/write error: address = 0x%.8x; write data = 0x%.8x; read data = 0x%.8x\r\n",
                           (unsigned int)(u32MemoryAddr + j * sizeof(m_au32ReadData[0])), (unsigned int)m_au32WriteData[j], (unsigned int)m_au32ReadData[j]);
            }
        }

        u32MemoryAddr += (DATA_BUF_LEN * sizeof(m_au32ReadData[0]));
        (void)memset(m_au32ReadData, 0, (DATA_BUF_LEN * sizeof(m_au32ReadData[0])));
        BSP_LED_Toggle(LED_BLUE);
    }

    return u32TestErrCnt;
}

/**
 * @brief  Main function of EXMC SDRAM project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    /* Initialize system clock: */
    BSP_CLK_Init();

    /* EXCLK 30MHz */
    CLK_SetClockDiv(CLK_BUS_EXCLK, CLK_EXCLK_DIV8);

    /* Initialize UART print */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);

    /* Initialize LED */
    BSP_IO_Init();
    BSP_LED_Init();

    /* Configure SRAM. */
    (void)BSP_IS62WV51216_Init();
    BSP_IS62WV51216_GetMemInfo(&m_u32StartAddr, &m_u32ByteSize);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_EFM | LL_PERIPH_SRAM);

    DDL_Printf("Memory start address: 0x%.8X \r\n", (unsigned int)m_u32StartAddr);
    DDL_Printf("Memory end   address: 0x%.8X \r\n", (unsigned int)(m_u32StartAddr + m_u32ByteSize - 1UL));
    DDL_Printf("Memory size  (Bytes): 0x%.8X \r\n\r\n", (unsigned int)m_u32ByteSize);

    for (;;) {
        m_u32TestCnt++;
        DDL_Printf("******** Write/read test times: %u ********\r\n", (unsigned int)m_u32TestCnt);

        /* Initialize test data. */
        InitTestData();

        /****************** Access data width: Byte *****************/
        m_u32ByteTestErrorCnt = MEMORY_Access8(m_u32StartAddr, m_u32ByteSize);
        DDL_Printf("     Byte read/write error data count: %u \r\n", (unsigned int)m_u32ByteTestErrorCnt);

        /****************** Access data width: Halfword *************/
        m_u32HalfwordTestErrorCnt = MEMORY_Access16(m_u32StartAddr, m_u32ByteSize / 2UL);
        DDL_Printf(" Halfword read/write error data count: %u \r\n", (unsigned int)m_u32HalfwordTestErrorCnt);

        /****************** Access data width: Word *****************/
        m_u32WordTestErrorCnt = MEMORY_Access32(m_u32StartAddr, m_u32ByteSize / 4UL);
        DDL_Printf("     Word read/write error data count: %u \r\n\r\n", (unsigned int)m_u32WordTestErrorCnt);

        /****************** Error check ******************/
        if ((m_u32ByteTestErrorCnt > 0UL) || (m_u32HalfwordTestErrorCnt > 0UL) || (m_u32WordTestErrorCnt > 0UL)) {
            BSP_LED_On(LED_RED);
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
