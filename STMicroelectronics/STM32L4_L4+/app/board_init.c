/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "board_init.h"

#include <stdio.h>

#include "tx_api.h"

#include "stm32l475e_iot01.h"
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_tsensor.h"

#include "wifi.h"

#include "main.h"

UART_HandleTypeDef UartHandle;

// Expose functions from STMCubeMX generation
extern SPI_HandleTypeDef hspi;
extern RNG_HandleTypeDef hrng;

void SystemClock_Config(void);

static void UART_Console_Init(void)
{
    UartHandle.Instance                    = USART1;
    UartHandle.Init.BaudRate               = 115200;
    UartHandle.Init.WordLength             = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits               = UART_STOPBITS_1;
    UartHandle.Init.Parity                 = UART_PARITY_NONE;
    UartHandle.Init.Mode                   = UART_MODE_TX_RX;
    UartHandle.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    BSP_COM_Init(COM1, &UartHandle);
}

static void Init_MEM1_Sensors(void)
{
    if (ACCELERO_OK != BSP_ACCELERO_Init())
    {
        printf("ERROR: BSP_ACCELERO_Init\r\n");
    }

    if (GYRO_OK != BSP_GYRO_Init())
    {
        printf("ERROR: BSP_GYRO_Init\r\n");
    }

    if (MAGNETO_OK != BSP_MAGNETO_Init())
    {
        printf("ERROR: BSP_MAGNETO_Init\r\n");
    }

    if (HSENSOR_OK != BSP_HSENSOR_Init())
    {
        printf("ERROR: BSP_HSENSOR_Init\r\n");
    }

    if (TSENSOR_OK != BSP_TSENSOR_Init())
    {
        printf("ERROR: BSP_TSENSOR_Init\r\n");
    }

    if (PSENSOR_OK != BSP_PSENSOR_Init())
    {
        printf("ERROR: BSP_PSENSOR_Init\r\n");
    }
}

void board_init(void)
{
    HAL_Init();

    // Configure the system clock
    SystemClock_Config();

    // Initialize all configured peripherals
    MX_GPIO_Init();
    MX_RNG_Init();

    UART_Console_Init();

    // Initialize LED
    BSP_LED_Init(LED_GREEN);

    // Discovery and Intialize all the Target's Features
    Init_MEM1_Sensors();
}

int hardware_rand(void)
{
    uint32_t rand;
    HAL_RNG_GenerateRandomNumber(&hrng, &rand);
    return rand;
}

uint32_t HAL_GetTick(void)
{
    return tx_time_get() * 10;
}

// EXTI line detection callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
        case GPIO_PIN_1:
            SPI_WIFI_ISR();
            break;
    }
}

// WiFi interrupt handle
void SPI3_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&hspi);
}
