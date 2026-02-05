/**
 *******************************************************************************
 * @file  usb/usb_dev_msc/source/sd_card.h
 * @brief This file contains all the functions prototypes of the Secure
 *        Digital Card.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2023-09-30       CDT             First version
 @endverbatim
 *******************************************************************************
 * Copyright (C) 2022-2025, Xiaohua Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by XHSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/zBSD-3-Clause
 *
 *******************************************************************************
 */
#ifndef __SD_CARD_H__
#define __SD_CARD_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll.h"
#include "ev_hc32f4a0_lqfp176_bsp.h"
#include "sd.h"

/**
 * @addtogroup HC32F4A0_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Dev_Msc
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

void SdCard_Config(stc_sd_handle_t *handle);
en_flag_status_t SdCard_GetInsertState(void);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __SD_CARD_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
