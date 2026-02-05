/**
 *******************************************************************************
 * @file  usb/usb_dev_msc/source/usb_bsp.c
 * @brief BSP function for USB example
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-10-31       CDT             Add USB core ID select function
                                    Using micro SD card as memory and support USB HS
   2024-11-08       CDT             Re-config SRAM read/write wait cycle
                                    Optimize print information
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
#include "usb_bsp.h"
#include "hc32_ll.h"
#include "usb_dev_int.h"

/**
 * @addtogroup HC32F4A0_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Dev_Msc
 * @{
 */

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* unlock/lock peripheral */
#define EXAMPLE_PERIPH_WE               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
                                         LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
#define EXAMPLE_PERIPH_WP               (LL_PERIPH_EFM | LL_PERIPH_FCG | LL_PERIPH_SRAM)

/* USBFS Core*/
#define USBF_DP_PORT            (GPIO_PORT_A)
#define USBF_DP_PIN             (GPIO_PIN_12)
#define USBF_DM_PORT            (GPIO_PORT_A)
#define USBF_DM_PIN             (GPIO_PIN_11)
#define USBF_VBUS_PORT          (GPIO_PORT_A)
#define USBF_VBUS_PIN           (GPIO_PIN_09)
//#define USBF_SOF_PORT           (GPIO_PORT_A)
//#define USBF_SOF_PIN            (GPIO_PIN_08)

/* USBHS Core, embedded PHY */
#define USBH_DP_PORT            (GPIO_PORT_B)
#define USBH_DP_PIN             (GPIO_PIN_15)
#define USBH_DM_PORT            (GPIO_PORT_B)
#define USBH_DM_PIN             (GPIO_PIN_14)
#define USBH_VBUS_PORT          (GPIO_PORT_B)
#define USBH_VBUS_PIN           (GPIO_PIN_13)
#define USBH_SOF_PORT           (GPIO_PORT_A)
#define USBH_SOF_PIN            (GPIO_PIN_04)

/* USBHS Core, external PHY */
#define USBH_ULPI_CLK_PORT      (GPIO_PORT_E)
#define USBH_ULPI_CLK_PIN       (GPIO_PIN_12)
#define USBH_ULPI_DIR_PORT      (GPIO_PORT_C)
#define USBH_ULPI_DIR_PIN       (GPIO_PIN_02)
#define USBH_ULPI_NXT_PORT      (GPIO_PORT_C)
#define USBH_ULPI_NXT_PIN       (GPIO_PIN_03)
#define USBH_ULPI_STP_PORT      (GPIO_PORT_C)
#define USBH_ULPI_STP_PIN       (GPIO_PIN_00)
#define USBH_ULPI_D0_PORT       (GPIO_PORT_E)
#define USBH_ULPI_D0_PIN        (GPIO_PIN_13)
#define USBH_ULPI_D1_PORT       (GPIO_PORT_E)
#define USBH_ULPI_D1_PIN        (GPIO_PIN_14)
#define USBH_ULPI_D2_PORT       (GPIO_PORT_E)
#define USBH_ULPI_D2_PIN        (GPIO_PIN_15)
#define USBH_ULPI_D3_PORT       (GPIO_PORT_B)
#define USBH_ULPI_D3_PIN        (GPIO_PIN_10)
#define USBH_ULPI_D4_PORT       (GPIO_PORT_B)
#define USBH_ULPI_D4_PIN        (GPIO_PIN_11)
#define USBH_ULPI_D5_PORT       (GPIO_PORT_B)
#define USBH_ULPI_D5_PIN        (GPIO_PIN_12)
#define USBH_ULPI_D6_PORT       (GPIO_PORT_B)
#define USBH_ULPI_D6_PIN        (GPIO_PIN_13)
#define USBH_ULPI_D7_PORT       (GPIO_PORT_E)
#define USBH_ULPI_D7_PIN        (GPIO_PIN_11)
/* 3300 reset */
#define USB_3300_RESET_PORT     (EIO_PORT1)
#define USB_3300_RESET_PIN      (EIO_USB3300_RST)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
extern usb_core_instance usb_dev;

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
 * @brief  handle the USB interrupt
 * @param  None
 * @retval None
 */
static void USB_IRQ_Handler(void)
{
    usb_isr_handler(&usb_dev);
}

/**
 * @brief  BSP clock initialize.
 *         SET board system clock to PLL@200MHz
 * @param  None
 * @retval None
 */
void BSP_CLK_Init(void)
{
    stc_clock_xtal_init_t stcXtalInit;
    stc_clock_pll_init_t stcPLLHInit;
    stc_clock_pllx_init_t stcPLLAInit;

    CLK_SetClockDiv(CLK_BUS_CLK_ALL,
                    (CLK_PCLK0_DIV1 | CLK_PCLK1_DIV2 | CLK_PCLK2_DIV4 |
                     CLK_PCLK3_DIV4 | CLK_PCLK4_DIV2 | CLK_EXCLK_DIV2 |
                     CLK_HCLK_DIV1));
    (void)CLK_XtalStructInit(&stcXtalInit);
    /* Config Xtal and enable Xtal */
    stcXtalInit.u8Mode   = CLK_XTAL_MD_OSC;
    stcXtalInit.u8Drv    = CLK_XTAL_DRV_ULOW;
    stcXtalInit.u8State  = CLK_XTAL_ON;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_2MS;

    GPIO_AnalogCmd(BSP_XTAL_PORT, BSP_XTAL_PIN, ENABLE);
    (void)CLK_XtalInit(&stcXtalInit);

    (void)CLK_PLLStructInit(&stcPLLHInit);
    /* VCO = (8/1)*100 = 800MHz*/
    stcPLLHInit.u8PLLState      = CLK_PLL_ON;
    stcPLLHInit.PLLCFGR         = 0UL;
    stcPLLHInit.PLLCFGR_f.PLLM  = 1UL - 1UL;
    stcPLLHInit.PLLCFGR_f.PLLN  = 100UL - 1UL;
    stcPLLHInit.PLLCFGR_f.PLLP  = 4UL - 1UL;
    stcPLLHInit.PLLCFGR_f.PLLQ  = 4UL - 1UL;
    stcPLLHInit.PLLCFGR_f.PLLR  = 4UL - 1UL;
    stcPLLHInit.PLLCFGR_f.PLLSRC = CLK_PLL_SRC_XTAL;
    (void)CLK_PLLInit(&stcPLLHInit);

    /* SRAM Read/Write wait cycle setting */
    SRAM_SetWaitCycle(SRAM_SRAMH, SRAM_WAIT_CYCLE0, SRAM_WAIT_CYCLE0);
    SRAM_SetWaitCycle((SRAM_SRAM123 | SRAM_SRAM4 | SRAM_SRAMB), SRAM_WAIT_CYCLE1, SRAM_WAIT_CYCLE1);
    /* 0-wait @ 40MHz */
    EFM_SetWaitCycle(EFM_WAIT_CYCLE5);
    /* 4 cycles for 200 ~ 250MHz */
    GPIO_SetReadWaitCycle(GPIO_RD_WAIT4);
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_PLL);

    /* PLLX for USB */
    (void)CLK_PLLxStructInit(&stcPLLAInit);
    /* VCO = (8/2)*120 = 480MHz*/
    stcPLLAInit.u8PLLState = CLK_PLL_ON;
    stcPLLAInit.PLLCFGR = 0UL;
    stcPLLAInit.PLLCFGR_f.PLLM = 2UL - 1UL;
    stcPLLAInit.PLLCFGR_f.PLLN = 120UL - 1UL;
    stcPLLAInit.PLLCFGR_f.PLLP = 10UL - 1UL;
    stcPLLAInit.PLLCFGR_f.PLLQ = 4UL - 1UL;
    stcPLLAInit.PLLCFGR_f.PLLR = 4UL - 1UL;
    (void)CLK_PLLxInit(&stcPLLAInit);
}

/**
 * @brief  initialize configurations for the BSP
 * @param  [in] pdev                device instance
 * @param  [in] pstcPortIdentify    usb core and phy select
 * @retval None
 */
void usb_bsp_init(usb_core_instance *pdev, stc_usb_port_identify *pstcPortIdentify)
{
    stc_gpio_init_t stcGpioCfg;

    /* Unlock peripherals or registers */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);

    BSP_CLK_Init();
    BSP_IO_Init();
    BSP_LED_Init();

#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
#endif
    /* USB clock source configure */
    CLK_SetUSBClockSrc(CLK_USBCLK_PLLXP);

#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_Printf("USB start !!\r\n");
#endif
    (void)GPIO_StructInit(&stcGpioCfg);

    if (USBFS_CORE_ID == pstcPortIdentify->u8CoreID) {
#ifdef USB_FS_MODE
        stcGpioCfg.u16PinAttr = PIN_ATTR_ANALOG;
        (void)GPIO_Init(USBF_DM_PORT, USBF_DM_PIN, &stcGpioCfg);
        (void)GPIO_Init(USBF_DP_PORT, USBF_DP_PIN, &stcGpioCfg);
        GPIO_SetFunc(USBF_VBUS_PORT, USBF_VBUS_PIN, GPIO_FUNC_10); /* VBUS */
        FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USBFS, ENABLE);
#endif
    } else {
#ifdef USB_HS_MODE
        if (USBHS_PHY_EMBED == pstcPortIdentify->u8PhyType) {
            /* USBHS work in embedded PHY */
            stcGpioCfg.u16PinAttr = PIN_ATTR_ANALOG;
            (void)GPIO_Init(USBH_DM_PORT, USBH_DM_PIN, &stcGpioCfg);
            (void)GPIO_Init(USBH_DP_PORT, USBH_DP_PIN, &stcGpioCfg);
            GPIO_SetFunc(USBH_VBUS_PORT, USBH_VBUS_PIN, GPIO_FUNC_12);
        } else {
            /* Reset 3300 */
            BSP_IO_WritePortPin(USB_3300_RESET_PORT, USB_3300_RESET_PIN, EIO_PIN_SET);
            BSP_IO_ConfigPortPin(USB_3300_RESET_PORT, USB_3300_RESET_PIN, EIO_DIR_OUT);

            (void)GPIO_StructInit(&stcGpioCfg);
            /* High drive capability */
            stcGpioCfg.u16PinDrv = PIN_HIGH_DRV;
            (void)GPIO_Init(USBH_ULPI_D0_PORT, USBH_ULPI_D0_PIN, &stcGpioCfg);
            (void)GPIO_Init(USBH_ULPI_D1_PORT, USBH_ULPI_D1_PIN, &stcGpioCfg);
            (void)GPIO_Init(USBH_ULPI_D2_PORT, USBH_ULPI_D2_PIN, &stcGpioCfg);
            (void)GPIO_Init(USBH_ULPI_D3_PORT, USBH_ULPI_D3_PIN, &stcGpioCfg);
            (void)GPIO_Init(USBH_ULPI_D4_PORT, USBH_ULPI_D4_PIN, &stcGpioCfg);
            (void)GPIO_Init(USBH_ULPI_D5_PORT, USBH_ULPI_D5_PIN, &stcGpioCfg);
            (void)GPIO_Init(USBH_ULPI_D6_PORT, USBH_ULPI_D6_PIN, &stcGpioCfg);
            (void)GPIO_Init(USBH_ULPI_D7_PORT, USBH_ULPI_D7_PIN, &stcGpioCfg);
            (void)GPIO_Init(USBH_ULPI_STP_PORT, USBH_ULPI_STP_PIN, &stcGpioCfg);

            GPIO_SetFunc(USBH_ULPI_CLK_PORT, USBH_ULPI_CLK_PIN, GPIO_FUNC_10);
            GPIO_SetFunc(USBH_ULPI_DIR_PORT, USBH_ULPI_DIR_PIN, GPIO_FUNC_10);
            GPIO_SetFunc(USBH_ULPI_NXT_PORT, USBH_ULPI_NXT_PIN, GPIO_FUNC_10);
            GPIO_SetFunc(USBH_ULPI_STP_PORT, USBH_ULPI_STP_PIN, GPIO_FUNC_10);
            GPIO_SetFunc(USBH_ULPI_D0_PORT, USBH_ULPI_D0_PIN, GPIO_FUNC_10);
            GPIO_SetFunc(USBH_ULPI_D1_PORT, USBH_ULPI_D1_PIN, GPIO_FUNC_10);
            GPIO_SetFunc(USBH_ULPI_D2_PORT, USBH_ULPI_D2_PIN, GPIO_FUNC_10);
            GPIO_SetFunc(USBH_ULPI_D3_PORT, USBH_ULPI_D3_PIN, GPIO_FUNC_10);
            GPIO_SetFunc(USBH_ULPI_D4_PORT, USBH_ULPI_D4_PIN, GPIO_FUNC_10);
            GPIO_SetFunc(USBH_ULPI_D5_PORT, USBH_ULPI_D5_PIN, GPIO_FUNC_10);
            GPIO_SetFunc(USBH_ULPI_D6_PORT, USBH_ULPI_D6_PIN, GPIO_FUNC_10);
            GPIO_SetFunc(USBH_ULPI_D7_PORT, USBH_ULPI_D7_PIN, GPIO_FUNC_10);

            BSP_IO_WritePortPin(USB_3300_RESET_PORT, USB_3300_RESET_PIN, EIO_PIN_RESET);
        }
        FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USBHS, ENABLE);
#endif
    }
}

/**
 * @brief  configure the NVIC of USB
 * @param  [in] pdev                    device instance
 * @retval None
 */
void usb_bsp_nvicconfig(usb_core_instance *pdev)
{
    stc_irq_signin_config_t stcIrqRegiConf;
    /* Register INT_SRC_USBFS_GLB Int to Vect.No.030 */
    stcIrqRegiConf.enIRQn = INT030_IRQn;
    /* Select interrupt function */
#ifdef USB_FS_MODE
    stcIrqRegiConf.enIntSrc = INT_SRC_USBFS_GLB;
#else
    stcIrqRegiConf.enIntSrc = INT_SRC_USBHS_GLB;
#endif
    /* Callback function */
    stcIrqRegiConf.pfnCallback = &USB_IRQ_Handler;
    /* Registration IRQ */
    (void)INTC_IrqSignIn(&stcIrqRegiConf);
    /* Clear Pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIO_15);
    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
}

/**
 * @brief  This function provides delay time in micro sec
 * @param  [in] usec         Value of delay required in micro sec
 * @retval None
 */
void usb_udelay(const uint32_t usec)
{
    __IO uint32_t i;
    uint32_t j;
    j = (HCLK_VALUE + 1000000UL - 1UL) / 1000000UL * usec;
    for (i = 0UL; i < j; i++) {
    }
}

/**
 * @brief  This function provides delay time in milli sec
 * @param  [in] msec         Value of delay required in milli sec
 * @retval None
 */
void usb_mdelay(const uint32_t msec)
{
    usb_udelay(msec * 1000UL);
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
