/**
 *******************************************************************************
 * @file  usb/usb_dev_hid_custom/source/usb_bsp.c
 * @brief BSP function for USB example
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-10-31       CDT             Add USB core ID select function
   2024-11-08       CDT             Modify timer0 clock division for asynchronous clock
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
#include "usb_dev_custom_hid_class.h"
#include "usb_dev_driver.h"
#include "usb_dev_int.h"

/**
 * @addtogroup HC32F4A0_DDL_Applications
 * @{
 */

/**
 * @addtogroup USB_Dev_Hid_Custom
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

/* KEY10 */
#define KEY10_PORT              (GPIO_PORT_A)
#define KEY10_PIN               (GPIO_PIN_00)
#define KEY10_EXTINT_CH         (EXTINT_CH00)
#define KEY10_INT_SRC           (INT_SRC_PORT_EIRQ0)
#define KEY10_IRQn              (INT003_IRQn)

/* USBFS Core*/
#define USBF_DP_PORT            (GPIO_PORT_A)
#define USBF_DP_PIN             (GPIO_PIN_12)
#define USBF_DM_PORT            (GPIO_PORT_A)
#define USBF_DM_PIN             (GPIO_PIN_11)
#define USBF_VBUS_PORT          (GPIO_PORT_A)
#define USBF_VBUS_PIN           (GPIO_PIN_09)
#define USBF_SOF_PORT           (GPIO_PORT_A)
#define USBF_SOF_PIN            (GPIO_PIN_08)

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

#define TMR0x                   (CM_TMR0_1)
#define TMR0_CLK                (FCG2_PERIPH_TMR0_1)
#define TMR0_CH_x               (TMR0_CH_A)
#define TMR0_INT_TYPE           (TMR0_INT_CMP_A)
#define TMR0_FLAG               (TMR0_FLAG_CMP_A)
/* TMR0 interrupt source and number define */
#define TMR0_IRQn               (INT014_IRQn)
#define TMR0_SOURCE             (INT_SRC_TMR0_1_CMP_A)

#define TMR0_CLK_SRC            (TMR0_CLK_SRC_LRC)
#define TMR0_CLK_DIV            (TMR0_CLK_DIV4)
#define TMR0_CMP_VAL            (32768UL/4UL)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
uint8_t PrevXferDone = 1U;

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
extern  usb_core_instance usb_dev;
extern  void usb_isr_handler(usb_core_instance *pdev);

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
 * @brief  KEY10 External interrupt Ch.0 callback function
 * @param  None
 * @retval None
 */
static void EXTINT_KEY10_IrqCallback(void)
{
    if (SET == EXTINT_GetExtIntStatus(KEY10_EXTINT_CH)) {
        if ((0U != PrevXferDone) && (usb_dev.dev.device_cur_status == USB_DEV_CONFIGURED)) {
            Send_Buf[0U] = KEY_REPORT_ID;
            if (PIN_RESET == GPIO_ReadInputPins(KEY10_PORT, KEY10_PIN)) {
                Send_Buf[1] = 0x01U;
            } else {
                Send_Buf[1] = 0x00U;
            }
            usb_deveptx(&usb_dev, HID_IN_EP, Send_Buf, 2);
            PrevXferDone = 0U;
        }
        EXTINT_ClearExtIntStatus(KEY10_EXTINT_CH);
    }
}

/**
 * @brief  configure the gpio related with the KEY10 and the NVIC
 * @param  None
 * @retval None
 */
static void Key10_Init(void)
{
    stc_extint_init_t stcExtIntInit;
    stc_irq_signin_config_t stcIrqSignConfig;
    stc_gpio_init_t stcGpioInit;

    /* GPIO config */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16ExtInt = PIN_EXTINT_ON;
    stcGpioInit.u16PullUp = PIN_PU_ON;
    (void)GPIO_Init(KEY10_PORT, KEY10_PIN, &stcGpioInit);

    /* ExtInt config */
    (void)EXTINT_StructInit(&stcExtIntInit);
    stcExtIntInit.u32Edge = EXTINT_TRIG_FALLING;
    (void)EXTINT_Init(KEY10_EXTINT_CH, &stcExtIntInit);

    /* IRQ sign-in */
    stcIrqSignConfig.enIntSrc = KEY10_INT_SRC;
    stcIrqSignConfig.enIRQn   = KEY10_IRQn;
    stcIrqSignConfig.pfnCallback = &EXTINT_KEY10_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);

    /* NVIC config */
    NVIC_ClearPendingIRQ(KEY10_IRQn);
    NVIC_SetPriority(KEY10_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(KEY10_IRQn);
}

/**
 * @brief  TMR0_1 channelA compare IRQ callback
 * @param  None
 * @retval None
 */
static void TMR0_1_ChACmp_IrqCallback(void)
{
    if ((0U != PrevXferDone) && (usb_dev.dev.device_cur_status == USB_DEV_CONFIGURED)) {
        usb_deveptx(&usb_dev, HID_IN_EP, NULL, 0);
        PrevXferDone = 0U;
    }
    /* Clear the compare matching flag */
    TMR0_ClearStatus(TMR0x, TMR0_FLAG);
}

/**
 * @brief  initialize the Timer0
 * @param  None
 * @retval None
 */
static void TMR0_Init_1S(void)
{
    stc_tmr0_init_t stcTmr0Init;
    stc_irq_signin_config_t stcIrqSignConfig;

    /* Enable timer0 peripheral clock */
    FCG_Fcg2PeriphClockCmd(TMR0_CLK, ENABLE);

    /* TIMER0 basetimer function initialize */
    (void)TMR0_StructInit(&stcTmr0Init);
    stcTmr0Init.u32ClockDiv = TMR0_CLK_DIV;        /* Config clock division */
    stcTmr0Init.u32ClockSrc = TMR0_CLK_SRC;          /* Chose clock source */
    stcTmr0Init.u32Func = TMR0_FUNC_CMP;            /* Timer0 compare mode */
    stcTmr0Init.u16CompareValue = (uint16_t)TMR0_CMP_VAL;             /* Set compara register data */
    (void)TMR0_Init(TMR0x, TMR0_CH_x, &stcTmr0Init);
    /* In asynchronous clock, If you want to write a TMR0 register, you need to wait for at
       least 6 asynchronous clock cycles after the last write operation! */
    DDL_DelayMS(1U); /* Wait at least 6 asynchronous clock cycles.*/
    /* Timer0 interrupt function Enable */
    TMR0_IntCmd(TMR0x, TMR0_INT_TYPE, ENABLE);
    DDL_DelayMS(1U); /* Wait at least 6 asynchronous clock cycles.*/

    /* Register IRQ handler && configure NVIC. */
    stcIrqSignConfig.enIRQn = TMR0_IRQn;
    stcIrqSignConfig.enIntSrc = TMR0_SOURCE;
    stcIrqSignConfig.pfnCallback = &TMR0_1_ChACmp_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

    /* Timer0 ch1 start counting */
    TMR0_Start(TMR0x, TMR0_CH_x);
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
    CLK_SetUSBClockSrc(CLK_USBCLK_SYSCLK_DIV5);
    /* KEY K10 interrupt function initialize */
    Key10_Init();
    /* Initlialize for 1S interrupt */
    TMR0_Init_1S();

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
