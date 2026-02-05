/**
 *******************************************************************************
 * @file  usart/usart_uart_halfduplex_polling/source/main.c
 * @brief This example demonstrates UART half-duplex data receive and transfer
 *        by polling mode.
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

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "main.h"

/**
 * @addtogroup HC32F4A0_DDL_Examples
 * @{
 */

/**
 * @addtogroup USART_UART_HalfDuplex_Polling
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

/* USART TX pin definition */
#define USART_TX_PORT                   (GPIO_PORT_E)   /* PE6: USART6_TX */
#define USART_TX_PIN                    (GPIO_PIN_06)
#define USART_TX_GPIO_FUNC              (GPIO_FUNC_36)

/* USART unit definition */
#define USART_UNIT                      (CM_USART6)
#define USART_FCG_ENABLE()              (FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_USART6, ENABLE))

/* Communication data size */
#define COM_DATA_LEN                    (ARRAY_SZ(m_au8TxData))

/* DDL_ON: master device / DDL_OFF: slave device */
#define MASTER_DEVICE_ENABLE            (DDL_ON)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static const uint8_t m_au8TxData[] = "USART half-duplux test.";
static uint8_t m_au8RxData[COM_DATA_LEN];

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @brief  Main function of UART halfduplex polling project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    stc_usart_uart_init_t stcUartInit;

    /* MCU Peripheral registers write unprotected */
    LL_PERIPH_WE(LL_PERIPH_SEL);

    /* Initialize BSP system clock. */
    BSP_CLK_Init();

    /* Initialize BSP expand IO. */
    BSP_IO_Init();

    /* Initialize BSP LED. */
    BSP_LED_Init();

    /* Initialize BSP key. */
    BSP_KEY_Init();

    /* Configure USART TX pin. */
    GPIO_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_GPIO_FUNC);

    /* Enable peripheral clock */
    USART_FCG_ENABLE();

    /* Initialize UART half-duplex. */
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32ClockDiv = USART_CLK_DIV64;
    stcUartInit.u32Baudrate = 9600UL;
    stcUartInit.u32OverSampleBit = USART_OVER_SAMPLE_8BIT;
    if (LL_OK != USART_HalfDuplex_Init(USART_UNIT, &stcUartInit, NULL)) {
        BSP_LED_On(LED_RED);
        for (;;) {
        }
    }

    /* MCU Peripheral registers write protected */
    LL_PERIPH_WP(LL_PERIPH_SEL);

#if (MASTER_DEVICE_ENABLE == DDL_ON)
    /* Wait key press */
    while (RESET == BSP_KEY_GetStatus(BSP_KEY_1)) {
    }

    /* Master send data */
    USART_FuncCmd(USART_UNIT, USART_TX, ENABLE);
    USART_UART_Trans(USART_UNIT, m_au8TxData, COM_DATA_LEN, USART_MAX_TIMEOUT);

    /* Master receive data */
    USART_FuncCmd(USART_UNIT, USART_TX, DISABLE);
    USART_FuncCmd(USART_UNIT, USART_RX, ENABLE);
    USART_UART_Receive(USART_UNIT, m_au8RxData, COM_DATA_LEN, USART_MAX_TIMEOUT);

#else
    /* Slave receive data */
    USART_FuncCmd(USART_UNIT, USART_RX, ENABLE);
    USART_UART_Receive(USART_UNIT, m_au8RxData, COM_DATA_LEN, USART_MAX_TIMEOUT);

    /* Slave send data */
    USART_FuncCmd(USART_UNIT, USART_RX, DISABLE);
    USART_FuncCmd(USART_UNIT, USART_TX, ENABLE);
    USART_UART_Trans(USART_UNIT, m_au8RxData, COM_DATA_LEN, USART_MAX_TIMEOUT);
#endif

    /* Compare data */
    if (0 == memcmp(m_au8RxData, m_au8TxData, COM_DATA_LEN)) {
        BSP_LED_On(LED_BLUE);
    } else {
        BSP_LED_On(LED_RED);
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
