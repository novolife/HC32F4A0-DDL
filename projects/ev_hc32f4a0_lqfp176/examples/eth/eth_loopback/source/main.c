/**
 *******************************************************************************
 * @file  eth/eth_loopback/source/main.c
 * @brief This example code implements a ethernet loopback function.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2023-09-30       CDT             SysTick_Handler add __DSB for Arm Errata 838869
   2024-11-08       CDT             Delete interrupt codes of RMII interface
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
 * @addtogroup ETH_Loopback
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

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static struct netif gnetif;
/* eth tx buffer */
static struct pbuf txPbuf;
static char txBuf[] = "Customers should select appropriate products for your \
                       application, and design, verify and test your application.";

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  SysTick interrupt callback function.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
    SysTick_IncTick();

    __DSB();  /* Arm Errata 838869 */
}

/**
 * @brief  Configure the BSP.
 * @param  None
 * @retval None
 */
static void BSP_Config(void)
{
    BSP_CLK_Init();
    BSP_IO_Init();
    BSP_LED_Init();
    BSP_KEY_Init();
    /* Configure UART */
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
    /* SysTick configuration */
    (void)SysTick_Init(1000U);
    /* Set Systick Interrupt to the highest priority */
    NVIC_SetPriority(SysTick_IRQn, DDL_IRQ_PRIO_00);
}

/**
 * @brief  Notify link status change.
 * @param  [in] netif                   Pointer to a struct netif structure
 * @retval None
 */
void EthernetIF_NotifyLinkChange(struct netif *netif)
{
    if (LL_OK == EthernetIF_IsLinkUp(netif)) {
        BSP_LED_Off(LED_RED);
        BSP_LED_On(LED_BLUE);
    } else {
        BSP_LED_Off(LED_BLUE);
        BSP_LED_On(LED_RED);
    }
}

/**
 * @brief  Input data handle callback.
 * @param  netif                        The network interface structure for this ethernetif.
 * @param  p                            The MAC packet to receive.
 * @retval None
 */
void EthernetIF_InputCallback(struct netif *netif, struct pbuf *p)
{
    if ((0 == (memcmp(p->payload, txPbuf.payload, p->len))) && (p->len == txPbuf.len)) {
        BSP_LED_On(LED_YELLOW);
    } else {
        DDL_Printf("eth receive data error! \r\n");
    }
}

/**
 * @brief  Main function of ETH Loopback.
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
    /* Configure BSP */
    BSP_Config();
    /* Configure the Ethernet */
    (void)ethernetif_init(&gnetif);
    /* Peripheral registers write protected */
    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);
    /* fill data to txPbuf */
    txPbuf.next    = NULL;
    txPbuf.payload = txBuf;
    txPbuf.len     = strlen(txBuf);

    for (;;) {
        if (SET == BSP_KEY_GetStatus(BSP_KEY_2)) {
            BSP_LED_Off(LED_YELLOW);
            if (LL_OK != low_level_output(&gnetif, &txPbuf)) {
                DDL_Printf("eth send data error! \r\n");
            }
        }
        /* Read a received packet */
        ethernetif_input(&gnetif);
        /* Handle periodic timers */
        EthernetIF_PeriodicHandle(&gnetif);
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
