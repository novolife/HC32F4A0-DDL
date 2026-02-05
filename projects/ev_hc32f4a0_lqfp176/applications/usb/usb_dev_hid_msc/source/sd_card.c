/**
 *******************************************************************************
 * @file  usb/usb_dev_hid_msc/source/sd_card.c
 * @brief This file provides firmware functions to manage the Secure Digital Card.
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
 *                    opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
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

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/* SD transfer mode */
#define SD_TRANS_MD_POLLING             (0U)
#define SD_TRANS_MD_INT                 (1U)
#define SD_TRANS_MD_DMA                 (2U)
/* Populate the following macro with an value, reference "SD transfer mode" */
#define SD_TRANS_MD                     (SD_TRANS_MD_POLLING)

/* SDIOC DMA configuration define */
#define SDIOC_DMA_UNIT                  (CM_DMA1)
#define SDIOC_DMA_CLK                   (FCG0_PERIPH_DMA1 | FCG0_PERIPH_AOS)
#define SDIOC_DMA_TX_CH                 (DMA_CH0)
#define SDIOC_DMA_RX_CH                 (DMA_CH1)
#define SDIOC_DMA_TX_TRIG_CH            (AOS_DMA1_0)
#define SDIOC_DMA_RX_TRIG_CH            (AOS_DMA1_1)
#define SDIOC_DMA_TX_TRIG_SRC           (EVT_SRC_SDIOC1_DMAW)
#define SDIOC_DMA_RX_TRIG_SRC           (EVT_SRC_SDIOC1_DMAR)

/* SDIOC configuration define */
#define SDIOC_SD_UINT                   (CM_SDIOC1)
#define SDIOC_SD_CLK                    (FCG1_PERIPH_SDIOC1)
#define SIDOC_SD_INT_SRC                (INT_SRC_SDIOC1_SD)
#define SIDOC_SD_IRQ                    (INT006_IRQn)
/* CK = PC12 */
#define SDIOC_CK_PORT                   (GPIO_PORT_C)
#define SDIOC_CK_PIN                    (GPIO_PIN_12)
/* CMD = PD02 */
#define SDIOC_CMD_PORT                  (GPIO_PORT_D)
#define SDIOC_CMD_PIN                   (GPIO_PIN_02)
/* D0 = PB07 */
#define SDIOC_D0_PORT                   (GPIO_PORT_B)
#define SDIOC_D0_PIN                    (GPIO_PIN_07)
/* D1 = PA08 */
#define SDIOC_D1_PORT                   (GPIO_PORT_A)
#define SDIOC_D1_PIN                    (GPIO_PIN_08)
/* D2 = PC10 */
#define SDIOC_D2_PORT                   (GPIO_PORT_C)
#define SDIOC_D2_PIN                    (GPIO_PIN_10)
/* D3 = PB05 */
#define SDIOC_D3_PORT                   (GPIO_PORT_B)
#define SDIOC_D3_PIN                    (GPIO_PIN_05)

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
 * @brief  Get SD card insert state.
 * @param  None
 * @retval An @ref en_flag_status_t enumeration type value.
 */
en_flag_status_t SdCard_GetInsertState(void)
{
    en_flag_status_t enFlagSta = SET;

    /* Check SD card detect pin */
    if (0U != BSP_IO_ReadPortPin(EIO_PORT0, EIO_SDIO1_CD)) {
        enFlagSta = RESET;
    }

    return enFlagSta;
}

/**
 * @brief  SD card configuration.
 * @param  None
 * @retval None
 */
void SdCard_Config(stc_sd_handle_t *handle)
{
#if SD_TRANS_MD != SD_TRANS_MD_POLLING
    stc_irq_signin_config_t stcIrqSignConfig;

    /* Interrupt configuration */
    stcIrqSignConfig.enIntSrc    = SIDOC_SD_INT_SRC;
    stcIrqSignConfig.enIRQn      = SIDOC_SD_IRQ;
    stcIrqSignConfig.pfnCallback = &SdCard_TransCompleteIrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);
#endif

    /* Enable SDIOC clock */
    FCG_Fcg1PeriphClockCmd(SDIOC_SD_CLK, ENABLE);
    /* SDIOC pins configuration */
    BSP_IO_ConfigPortPin(EIO_PORT0, EIO_SDIO1_CD, EIO_DIR_IN);
    GPIO_SetFunc(SDIOC_CK_PORT,  SDIOC_CK_PIN,  GPIO_FUNC_9);
    GPIO_SetFunc(SDIOC_CMD_PORT, SDIOC_CMD_PIN, GPIO_FUNC_9);
    GPIO_SetFunc(SDIOC_D0_PORT,  SDIOC_D0_PIN,  GPIO_FUNC_9);
    GPIO_SetFunc(SDIOC_D1_PORT,  SDIOC_D1_PIN,  GPIO_FUNC_9);
    GPIO_SetFunc(SDIOC_D2_PORT,  SDIOC_D2_PIN,  GPIO_FUNC_9);
    GPIO_SetFunc(SDIOC_D3_PORT,  SDIOC_D3_PIN,  GPIO_FUNC_9);

    /* Configure structure initialization */
    handle->SDIOCx                     = SDIOC_SD_UINT;
    handle->stcSdiocInit.u32Mode       = SDIOC_MD_SD;
    handle->stcSdiocInit.u8CardDetect  = SDIOC_CARD_DETECT_CD_PIN_LVL;
    handle->stcSdiocInit.u8SpeedMode   = SDIOC_SPEED_MD_HIGH;
    handle->stcSdiocInit.u8BusWidth    = SDIOC_BUS_WIDTH_4BIT;
    handle->stcSdiocInit.u16ClockDiv   = SDIOC_CLK_DIV2;
#if SD_TRANS_MD == SD_TRANS_MD_DMA
    SdCard_DMAInit();
    handle->DMAx      = SDIOC_DMA_UNIT;
    handle->u8DmaTxCh = SDIOC_DMA_TX_CH;
    handle->u8DmaRxCh = SDIOC_DMA_RX_CH;
#else
    handle->DMAx    = NULL;
#endif
}

/**
 * @}
 */

/**
 * @}
 */

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
