/**
 *******************************************************************************
 * @file  eth/eth_twoboards/source/ethernetif.h
 * @brief Ethernet interface header file.
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
#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32f4xx.h"

/**
 * @addtogroup HC32F4A0_DDL_Examples
 * @{
 */

/**
 * @addtogroup ETH_Two_Boards
 * @{
 */

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
/**
 * @defgroup ETH_IF_Global_Types Ethernet Interface Global Types
 * @{
 */

/**
 * @brief Main packet buffer Structure Definition
 */
struct pbuf {
    struct pbuf *next;                  /* next pbuf in singly linked pbuf chain    */
    void        *payload;               /* pointer to the actual data in the buffer */
    uint32_t    len;                    /* length of this buffer                    */
};

/**
 * @}
 */

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup ETH_IF_Global_Macros Ethernet Interface Global Macros
 * @{
 */

/**
 * @}
 */

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
/**
 * @addtogroup ETH_IF_Global_Functions
 * @{
 */
int32_t ethernetif_init(void);
void    ethernetif_input(void);
int32_t low_level_output(struct pbuf *p);

void EthernetIF_InputCallback(struct pbuf *p);

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

#endif /* __ETHERNETIF_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
