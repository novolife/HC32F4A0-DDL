/**
 *******************************************************************************
 * @file  dvp/dvp_camera_display/source/main.c
 * @brief This example demonstrates DVP capture function.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2023-09-30       CDT             Re-implement code
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
 * @addtogroup DVP_Camera_Display
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Peripheral register WE/WP selection */
#define LL_PERIPH_SEL                   (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                                         LL_PERIPH_EFM | LL_PERIPH_SRAM)

/* LCD definition */
#define LCD_RAM_ADDR                    (0x60002000UL)
#define LCD_CURSOR_POS_X                (0U)
#define LCD_CURSOR_POS_Y                (150U)

/* DVP DMA definition */
#define DVP_DMA_FCG                     (FCG0_PERIPH_DMA1)
#define DVP_DMA_UNIT                    (CM_DMA1)
#define DVP_DMA_CH                      (DMA_CH0)
#define DVP_DMA_AOS_TARGET_SEL          (AOS_DMA1_0)
#define DVP_DMA_TIMEOUT_MAX             (0xFFFFFFUL)

/* Camera definition */
#define CAMERA_OUTPUT_WIDTH             (480UL)
#define CAMERA_OUTPUT_HEIGHT            (500UL)
#define CAMERA_OUTPUT_SIZE              (CAMERA_OUTPUT_WIDTH * CAMERA_OUTPUT_HEIGHT)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static uint16_t m_u16DvpFrameData[CAMERA_OUTPUT_SIZE];

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Initialize camera.
 * @param  None
 * @retval None
 */
static void CAM_Init(void)
{
    /* Initialize OV5640 */
    BSP_OV5640_Init();

    /* Set OV5640 mode: RGB565 */
    BSP_OV5640_RGB565_Mode();

    /* Set OV5640 out size */
    BSP_OV5640_SetOutSize(0U, 0U, (uint16_t)CAMERA_OUTPUT_WIDTH, (uint16_t)CAMERA_OUTPUT_HEIGHT);
}

/**
 * @brief  Initialize LCD.
 * @param  None
 * @retval None
 */
static void LCD_Init(void)
{
    /* Initialize NT35510 LCD */
    BSP_NT35510_Init();

    /* Clear LCD screen */
    BSP_NT35510_Clear(LCD_COLOR_RED);

    /* Turn on LCD backlight */
    BSP_LCD_BKLCmd(EIO_PIN_SET);
}

/**
 * @brief  Initialize DMA for DVP.
 * @param  None
 * @retval None
 */
static void DVP_DMA_Init(void)
{
    stc_dma_init_t stcDmaInit;

    /* Enable DMA&AOS module clk */
    FCG_Fcg0PeriphClockCmd(DVP_DMA_FCG | FCG0_PERIPH_AOS, ENABLE);

    /*********************** DMA1_0 for DVP DMA request ***********************/
    /* Initialize DMA1_0 */
    (void)DMA_StructInit(&stcDmaInit);
    stcDmaInit.u32DestAddrInc = DMA_DEST_ADDR_INC;
    stcDmaInit.u32SrcAddr   = (uint32_t)(&CM_DVP->DMR);
    stcDmaInit.u32DestAddr  = (uint32_t)(&m_u16DvpFrameData[0]);
    stcDmaInit.u32DataWidth = DMA_DATAWIDTH_32BIT;
    stcDmaInit.u32TransCount  = 0UL;
    stcDmaInit.u32BlockSize = 1UL;
    (void)DMA_Init(DVP_DMA_UNIT, DVP_DMA_CH, &stcDmaInit);

    /* DMA trigger source for DVP DMA request */
    AOS_SetTriggerEventSrc(DVP_DMA_AOS_TARGET_SEL, EVT_SRC_DVP_DMAREQ);

    /*********************** Enable DMA ***************************************/
    DMA_Cmd(DVP_DMA_UNIT, ENABLE);
}

/**
 * @brief  Main function of DVP camera display project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    uint32_t i;
    stc_dvp_init_t stcDvpInit;
    en_flag_status_t enFrameEndFlag;
    en_functional_state_t enCaptureStatus;
    en_functional_state_t enLcdRefresh = DISABLE;
    uint32_t u32DmaTransAddr;
    const uint32_t u32DmaTransEndAddr = (uint32_t)(m_u16DvpFrameData + CAMERA_OUTPUT_SIZE);

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Configure clock */
    BSP_CLK_Init();
    CLK_SetClockDiv(CLK_BUS_EXCLK, CLK_EXCLK_DIV8);

    /* Initialize CAM/LCD IO */
    BSP_IO_Init();
    BSP_LED_Init();
    BSP_CAM_IO_Init();
    BSP_LCD_IO_Init();
    BSP_KEY_Init();

    /* HW Reset LCD */
    BSP_LCD_RSTCmd(EIO_PIN_RESET);
    BSP_CAM_RSTCmd(EIO_PIN_SET);    /* RST# to low */
    DDL_DelayMS(100UL);
    BSP_LCD_RSTCmd(EIO_PIN_SET);
    BSP_CAM_RSTCmd(EIO_PIN_RESET);  /* RST# to high */
    BSP_CAM_STBCmd(EIO_PIN_SET);    /* STB# to low */
    DDL_DelayMS(100UL);

    /* Initialize LCD */
    LCD_Init();

    /* Initialize CAM */
    CAM_Init();

    /* Enable DVP module clk */
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_DVP, ENABLE);

    /* Initialize DMA for DVP */
    DVP_DMA_Init();

    /* Initialize DVP */
    (void)DVP_StructInit(&stcDvpInit);
    stcDvpInit.u32PIXCLKPolarity = DVP_PIXCLK_RISING;
    stcDvpInit.u32CaptureMode  = DVP_CAPT_MD_SINGLE_FRAME;
    (void)DVP_Init(&stcDvpInit);

    /* Enable DVP */
    DVP_Cmd(ENABLE);

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

    for (;;) {
        enCaptureStatus = DVP_GetCaptureState();
        enFrameEndFlag = DVP_GetStatus(DVP_FLAG_FRAME_END);

        if ((DISABLE == enCaptureStatus) && (SET == enFrameEndFlag)) {
            /**************** DVP capture camera frame ************************/
            (void)DMA_SetDestAddr(DVP_DMA_UNIT, DVP_DMA_CH, (uint32_t)m_u16DvpFrameData);
            (void)DMA_ChCmd(DVP_DMA_UNIT, DVP_DMA_CH, ENABLE);

            /* Wait the frame end */
            DVP_ClearStatus(DVP_FLAG_ALL);
            while (RESET == DVP_GetStatus(DVP_FLAG_FRAME_END)) {
            }

            /* Start DVP Capture*/
            DVP_CaptureCmd(ENABLE);

            /* Wait the all data of frame  */
            for (i = 0UL; i < DVP_DMA_TIMEOUT_MAX; i++) {
                u32DmaTransAddr = DMA_GetDestAddr(DVP_DMA_UNIT, DVP_DMA_CH);
                if (u32DmaTransAddr >= u32DmaTransEndAddr) {
                    enLcdRefresh = ENABLE;
                    break;
                }
            }

            (void)DMA_ChCmd(DVP_DMA_UNIT, DVP_DMA_CH, DISABLE);

            /**************** LCD display camera frame ************************/
            if (ENABLE == enLcdRefresh) {
                /* Set LCD cursor */
                BSP_NT35510_SetCursor(LCD_CURSOR_POS_X, LCD_CURSOR_POS_Y);
                BSP_NT35510_PrepareWriteRAM();

                /* Write frame data */
                for (i = 0UL; i < CAMERA_OUTPUT_SIZE; i++) {
                    BSP_NT35510_WriteData(m_u16DvpFrameData[i]);
                }

                enLcdRefresh = DISABLE;
            }
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
