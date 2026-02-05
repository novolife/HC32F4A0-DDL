/**
 *******************************************************************************
 * @file  exmc/exmc_sdram_sram/source/exmc_switch.h
 * @brief This file contains the including files of EXMC switch function.
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
#ifndef __EXMC_SWITCH_H__
#define __EXMC_SWITCH_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll.h"

/**
 * @addtogroup EXMC_Switch
 * @{
 */

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup EXMC_Switch_Global_Macros EXMC_Switch Global Macros
 * @note 1. Don't use SRAM after switch to DMC
 *       2. Don't use SDRAM after switch to SMC
 * @{
 */

/**
 * @defgroup EXMC_Switch_Pin_Latch_Bit EXMC_Switch Pin Latch Bit
 * @{
 */
#define SMC_CS_LTE              (bCM_GPIO->PCRG10_b.LTE)
#define SMC_WE_LTE              (bCM_GPIO->PCRC0_b.LTE)
#define SMC_OE_LTE              (bCM_GPIO->PCRF11_b.LTE)

#define DMC_CS_LTE              (bCM_GPIO->PCRG9_b.LTE)
/**
 * @}
 */

/**
 * @defgroup EXMC_Switch_Pin_Latch EXMC_Switch Pin Latch
 * @{
 */
#define DMC_InterfaceLatchOn()                                                 \
do {                                                                           \
    DMC_CS_LTE = 1;                                                            \
} while (0)

#define DMC_InterfaceLatchOff()                                                \
do {                                                                           \
    DMC_CS_LTE = 0;                                                            \
} while (0)

#define SMC_InterfaceLatchOn()                                                 \
do {                                                                           \
    SMC_CS_LTE = 1;                                                            \
} while (0)

#define SMC_InterfaceLatchOff()                                                \
do {                                                                           \
    SMC_CS_LTE = 0;                                                            \
} while (0)

#define EXMC_READ_DUMMY(addr)                                                  \
do {                                                                           \
    g_u16ReadDummy = *(uint16_t *)(addr);                                      \
} while (0)
/**
 * @}
 */

/**
 * @defgroup EXMC_Switch_DMC EXMC_Switch DMC
 * @{
 */
/* DMC read dummy */
#define DMC_READ_DUMMY()        EXMC_READ_DUMMY(g_u32ReadDummyDmcAddr)

/* Enable DMC */
#define DMC_ENABLE()                                                           \
do {                                                                           \
    WRITE_REG32(bCM_PERIC->DMC_ENAR_b.DMCEN, ENABLE);                          \
    DMC_InterfaceLatchOff();                                                   \
} while (0)

/* Disable DMC */
#define DMC_DISABLE()                                                          \
do {                                                                           \
    DMC_InterfaceLatchOn();                                                    \
    WRITE_REG32(bCM_PERIC->DMC_ENAR_b.DMCEN, DISABLE);                         \
} while (0)
/**
 * @}
 */

/**
 * @defgroup EXMC_Switch_SMC EXMC_Switch SMC
 * @{
 */
/* DMC read dummy */
#define SMC_READ_DUMMY()        EXMC_READ_DUMMY(g_u32ReadDummySmcAddr)

/* Enable SMC */
#define SMC_ENABLE()                                                           \
do {                                                                           \
    WRITE_REG32(bCM_PERIC->SMC_ENAR_b.SMCEN, ENABLE);                          \
    SMC_InterfaceLatchOff();                                                   \
} while (0)

/* Disable SMC */
#define SMC_DISABLE()                                                          \
do {                                                                           \
    SMC_InterfaceLatchOn();                                                    \
    WRITE_REG32(bCM_PERIC->SMC_ENAR_b.SMCEN, DISABLE);                         \
} while (0)
/**
 * @}
 */

/**
 * @defgroup EXMC_Switch_SDRAM EXMC_Switch SDRAM
 * @{
 */
/**
 * @brief  SDRAM enter low power.
 * @note   Switch state: 1. Ready to Pause
 *                       2. Pause to Low power
 */
#define SDRAM_EnterLowPower()                                                  \
do {                                                                           \
    WRITE_REG32(CM_DMC->STCR, EXMC_DMC_CTRL_STATE_PAUSE);                      \
    while (EXMC_DMC_CURR_STATUS_PAUSED != READ_REG32_BIT(CM_DMC->STSR, DMC_STSR_STATUS)) \
    {}                                                                         \
                                                                               \
    WRITE_REG32(CM_DMC->STCR, EXMC_DMC_CTRL_STATE_SLEEP);                      \
    while (EXMC_DMC_CURR_STATUS_LOWPOWER != READ_REG32_BIT(CM_DMC->STSR, DMC_STSR_STATUS)) \
    {}                                                                         \
} while (0)

/**
 * @brief  SDRAM exit low power.
 * @note   Switch state: 1. Low power to Pause
 *                       2. Pause to Ready
 */
#define SDRAM_ExitLowPower()                                                   \
do {                                                                           \
    WRITE_REG32(CM_DMC->STCR, EXMC_DMC_CTRL_STATE_WAKEUP);                     \
    while (EXMC_DMC_CURR_STATUS_PAUSED != READ_REG32_BIT(CM_DMC->STSR, DMC_STSR_STATUS)) \
    {}                                                                         \
                                                                               \
    WRITE_REG32(CM_DMC->STCR, EXMC_DMC_CTRL_STATE_GO);                         \
    while (EXMC_DMC_CURR_STATUS_RDY != READ_REG32_BIT(CM_DMC->STSR, DMC_STSR_STATUS)) \
    {}                                                                         \
} while (0)
/**
 * @}
 */

/**
 * @defgroup EXMC_Switch_Function EXMC_Switch Function
 * @{
 */
#define EXMC_SWITCH_SMC_TO_DMC()                                               \
do {                                                                           \
    SMC_READ_DUMMY();   /* Read_Dummy ensure that SRAM write completely. */    \
                                                                               \
    SMC_DISABLE();      /* Disable SMC */                                      \
                                                                               \
    DMC_ENABLE();       /* Enable DMC */                                       \
                                                                               \
    SDRAM_ExitLowPower();                                                      \
} while (0)

#define EXMC_SWITCH_DMC_TO_SMC()                                               \
do {                                                                           \
    DMC_READ_DUMMY();   /* Read_Dummy ensure that SDRAM write completely. */   \
                                                                               \
    SDRAM_EnterLowPower();                                                     \
                                                                               \
    DMC_DISABLE();      /* Disable DMC */                                      \
                                                                               \
    SMC_ENABLE();       /* Enable DMC */                                       \
} while (0)
/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/
/**
 * @addtogroup EXMC_SWITCH_Global_Variables
 * @{
 */
extern __IO uint16_t g_u16ReadDummy;
extern uint32_t g_u32ReadDummyDmcAddr;
extern uint32_t g_u32ReadDummySmcAddr;
/**
 * @}
 */

/*******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/
/**
 * @addtogroup EXMC_SWITCH_Global_Functions
 * @{
 */
__STATIC_INLINE void EXMC_SetDmcReadDummyAddr(uint32_t u32Addr)
{
    g_u32ReadDummyDmcAddr = u32Addr;
}

__STATIC_INLINE void EXMC_SetSmcReadDummyAddr(uint32_t u32Addr)
{
    g_u32ReadDummySmcAddr = u32Addr;
}
/**
 * @}
 */

/**
 * @}
 */

#endif /* __EXMC_SWITCH_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
