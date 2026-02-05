/**
 *******************************************************************************
 * @file  usb/usb_dev_hid_msc/source/usb_dev_msc_msd.c
 * @brief user MSC application layer.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-10-31       CDT             Modify Vendor Identification
                                    Using micro SD card as memory and support USB HS
   2023-09-30       CDT             Do not erase SD card before write operation
   2024-11-08       CDT             Update for new SDIOC midwares
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
#include "usb_dev_msc_msd.h"
#include "usb_dev_msc_mem.h"
#include "ev_hc32f4a0_lqfp176_bsp.h"
#include "sd_card.h"

/**
 * @addtogroup HC32F4A0_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Dev_Hid_Msc
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
int8_t msc_init(uint8_t lun);
int8_t msc_getcapacity(uint8_t lun, uint32_t *block_num, uint32_t *block_size);
int8_t msc_ifready(uint8_t lun);
int8_t msc_ifwrprotected(uint8_t lun);
int8_t msc_read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t msc_write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t msc_getmaxlun(void);

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define SD_TIMEOUT_CNT          (2000UL)

#define MSC_STATUS_BIT_SD_INI   (0x01U)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
/* Variable for Storage operation status */
__IO static uint8_t u8MscStatusReg = 0U;

/* USB Mass storage querty data (36 bytes for each lun) */
const int8_t msc_inquirydata[] = {
    /* LUN 0 */
    0x00,
    0x80,
    0x02,
    0x02,
    (USB_DEV_INQUIRY_LENGTH - 4U),
    0x00,
    0x00,
    0x00,
    /* Vendor Identification */
    'X', 'H', 'S', 'C', ' ', 'M', 'C', 'U', ' ',    /* 9 bytes */
    /* Product Identification */
    'M', 'i', 'c', 'r', 'o', ' ', 'S', 'D', ' ',    /* 15 bytes */
    ' ', 'D', 'i', 's', 'k', ' ',
    /* Product Revision Level */
    '1', '.', '0', ' ',                             /* 4 bytes */
    /* LUN 1 */
    0x00,
    0x80,
    0x02,
    0x02,
    (USB_DEV_INQUIRY_LENGTH - 4U),
    0x00,
    0x00,
    0x00,
    /* Vendor Identification */
    'X', 'H', 'S', 'C', ' ', 'M', 'C', 'U', ' ',    /* 9 bytes */
    /* Product Identification */
    'M', 'i', 'c', 'r', 'o', ' ', 'S', 'D', ' ',    /* 15 bytes */
    'D', 'i', 's', 'k', ' ', ' ',
    /* Product Revision Level */
    '1', '.', '0', ' ',                             /* 4 bytes */
};

static USB_DEV_MSC_cbk_TypeDef flash_fops = {
    &msc_init,
    &msc_getcapacity,
    &msc_getmaxlun,
    &msc_ifready,
    &msc_read,
    &msc_write,
    &msc_ifwrprotected,
    (int8_t *)msc_inquirydata
};

/* Pointer to flash_fops */
USB_DEV_MSC_cbk_TypeDef *msc_fops = &flash_fops;

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static stc_sd_card_info_t stcSdCardInfo;
static stc_sd_handle_t SdHandle;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  initialize storage
 * @param  [in] lun          logic number
 * @retval status
 */
int8_t msc_init(uint8_t lun)
{
    int8_t res = (int8_t)0;
    SdCard_Config(&SdHandle);
    u8MscStatusReg &= ~MSC_STATUS_BIT_SD_INI;

    /* Reset and init SDIOC */
    if (LL_OK != SDIOC_SWReset(SdHandle.SDIOCx, SDIOC_SW_RST_ALL)) {
        DDL_Printf("Reset SDIOC failed!\r\n");
    } else if (SET != SdCard_GetInsertState()) {
        DDL_Printf("No SD card insertion!\r\n");
    } else {
        if (LL_OK == SD_Init(&SdHandle)) {
            u8MscStatusReg |= MSC_STATUS_BIT_SD_INI;
            DDL_Printf("SD card initialize success!\r\n");
        } else {
            u8MscStatusReg &= ~MSC_STATUS_BIT_SD_INI;
            DDL_Printf("SD card initialize failed!\r\n");
        }
    }
    return res;
}

/**
 * @brief  Get SD memory status
 * @param  None
 * @retval status
 */
static int8_t JudgeSDStatus(void)
{
    int8_t res = (int8_t)0;
    if (RESET == SdCard_GetInsertState()) {
        /* SD disconnect */
        res = (int8_t) -1;
        /* SD card need initialization */
        u8MscStatusReg &= ~MSC_STATUS_BIT_SD_INI;
    } else {
        if (0U == (u8MscStatusReg & MSC_STATUS_BIT_SD_INI)) {
            /* SD need initialization */
            if (LL_OK != SDIOC_SWReset(SdHandle.SDIOCx, SDIOC_SW_RST_ALL)) {
                DDL_Printf("Reset SDIOC failed!\r\n");
            }
            if (LL_OK == SD_Init(&SdHandle)) {
                /* SD card had initialized */
                u8MscStatusReg |= MSC_STATUS_BIT_SD_INI;
                DDL_Printf("SD card initialize success!\r\n");
            } else {
                u8MscStatusReg &= ~MSC_STATUS_BIT_SD_INI;
                DDL_Printf("SD card initialize failed!\r\n");
                res = (int8_t) -1;
            }
        }
    }
    return res;
}

/**
 * @brief  Get Storage capacity
 * @param  [in] lun          logic number
 * @param  [in] block_num    sector number
 * @param  [in] block_size   sector size
 * @retval status
 */
int8_t msc_getcapacity(uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
    int8_t res;
    *block_size = 512U;
    *block_num  = 0U;

    res = JudgeSDStatus();

    if ((int8_t)0 == res) {
        (void)SD_GetCardInfo(&SdHandle, &stcSdCardInfo);
        *block_num  = stcSdCardInfo.u32BlockNum;
    }
    return res;
}

/**
 * @brief  Check if storage is ready
 * @param  [in] lun          logic number
 * @retval status
 */
int8_t  msc_ifready(uint8_t lun)
{
    int8_t res;
    en_sd_card_state_t enSdStd;

    res = JudgeSDStatus();

    if ((int8_t)0 == res) {
        (void)SD_GetCardState(&SdHandle, &enSdStd);
        if (SD_CARD_STAT_DISCONNECT == enSdStd) {
            res = (int8_t) -1;
        }
    }

    return res;
}

/**
 * @brief  Check if storage is write protected
 * @param  [in] lun          logic number
 * @retval status
 */
int8_t msc_ifwrprotected(uint8_t lun)
{
    return LL_OK;
}

/**
 * @brief  read data from storage devices
 * @param  [in] lun          logic number
 * @param  [in] buf          data buffer be read
 * @param  [in] blk_addr     sector address
 * @param  [in] blk_len      sector count
 * @retval status
 */
int8_t msc_read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    int8_t res;

    res = JudgeSDStatus();

    if ((int8_t)0 == res) {
        (void)SD_ReadBlocks(&SdHandle, blk_addr, blk_len, buf, SD_TIMEOUT_CNT);
    }

    return res;
}

/**
 * @brief  Write data to storage devices
 * @param  [in] lun          logic number
 * @param  [in] buf          data buffer be written
 * @param  [in] blk_addr     sector address
 * @param  [in] blk_len      sector count
 * @retval status
 */
int8_t msc_write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    int8_t res;

    res = JudgeSDStatus();

    if ((int8_t)0 == res) {
        (void)SD_WriteBlocks(&SdHandle, blk_addr, blk_len, buf, SD_TIMEOUT_CNT);
    }
    return res;
}

/**
 * @brief  Get supported logic number
 * @param  None
 * @retval max lun
 */
int8_t msc_getmaxlun(void)
{
    /* One LUN only */
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
