/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "board_init.h"

#include <stdio.h>

#include "tx_api.h"

#include "stm32l475e_iot01.h"
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_magneto.h"

#include "wifi.h"

UART_HandleTypeDef UartHandle;
RTC_HandleTypeDef RtcHandle;
TIM_HandleTypeDef TimCCHandle;
extern SPI_HandleTypeDef hspi;

volatile uint32_t ButtonPressed = 0;
volatile uint32_t SendData = 0;

static uint32_t t_TIM_CC1_Pulse;

// 2kHz/0.5 For Sensors Data data@0.5Hz
#define DEFAULT_TIM_CC1_PULSE 4000

// Defines related to Clock configuration
#define RTC_ASYNCH_PREDIV 0x7F  // LSE as RTC clock
#define RTC_SYNCH_PREDIV 0x00FF // LSE as RTC clock

#define CFG_HW_UART1_BAUDRATE 115200
#define CFG_HW_UART1_WORDLENGTH UART_WORDLENGTH_8B
#define CFG_HW_UART1_STOPBITS UART_STOPBITS_1
#define CFG_HW_UART1_PARITY UART_PARITY_NONE
#define CFG_HW_UART1_HWFLOWCTL UART_HWCONTROL_NONE
#define CFG_HW_UART1_MODE UART_MODE_TX_RX
#define CFG_HW_UART1_ADVFEATUREINIT UART_ADVFEATURE_NO_INIT

static void STM32_Error_Handler(void);
static void Init_MEM1_Sensors(void);
static void SystemClock_Config(void);
static void InitTimers(void);
static void InitRTC(void);
static void UART_Console_Init(void);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void SPI3_IRQHandler(void);

void board_init(void)
{
    // Init Platform
    HAL_Init();

    // Configure the System clock
    SystemClock_Config();

    // Initialize Real Time Clock
    InitRTC();

    // Initialize console
    UART_Console_Init();

    // Initialize button
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

    // Initialize LED
    BSP_LED_Init(LED2);

    // Discovery and Intialize all the Target's Features
    Init_MEM1_Sensors();

    // Initialize timers
    InitTimers();
}

static void Init_MEM1_Sensors(void)
{
    // Accelero
    if (ACCELERO_OK != BSP_ACCELERO_Init())
    {
        printf("Error Accelero Sensor\r\n");
    }

    // Gyro
    if (GYRO_OK != BSP_GYRO_Init())
    {
        printf("Error Gyroscope Sensor\r\n");
    }

    // Mag
    if (MAGNETO_OK != BSP_MAGNETO_Init())
    {
        printf("Error Magneto Sensor\r\n");
    }

    // Humidity
    if (HSENSOR_OK != BSP_HSENSOR_Init())
    {
        printf("Error Humidity Sensor\r\n");
    }

    // Temperature
    if (TSENSOR_OK != BSP_TSENSOR_Init())
    {
        printf("Error Temperature Sensor\r\n");
    }

    // Pressure
    if (PSENSOR_OK != BSP_PSENSOR_Init())
    {
        printf("Error Pressure Sensor\r\n");
    }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 48000000
  *            PLL_M                          = 6
  *            PLL_N                          = 20
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            PLL_Q                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit;

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 6;
    RCC_OscInitStruct.PLL.PLLN = 20;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        STM32_Error_Handler();
    }

    // Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        STM32_Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART3 | RCC_PERIPHCLK_I2C2 | RCC_PERIPHCLK_RNG;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInit.RngClockSelection = RCC_RNGCLKSOURCE_MSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        STM32_Error_Handler();
    }

    __HAL_RCC_PWR_CLK_ENABLE();

    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        STM32_Error_Handler();
    }

    // Enable MSI PLL mode
    HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief  Output Compare callback in non blocking mode
  * @param  htim : TIM OC handle
  * @retval None
  */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    uint32_t uhCapture = 0;

    // TIM1_CH1 toggling with frequency = 2Hz
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        uhCapture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        // Set the Capture Compare Register value
        __HAL_TIM_SET_COMPARE(&TimCCHandle, TIM_CHANNEL_1, (uhCapture + t_TIM_CC1_Pulse));
        SendData = 1;
        BSP_LED_Toggle(LED2);
    }
}

/**
* @brief  Function for initializing timers for sending the Telemetry data to IoT hub
 * @param  None
 * @retval None
 */
static void InitTimers(void)
{
    uint32_t uwPrescalerValue;

    // Timer Output Compare Configuration Structure declaration
    TIM_OC_InitTypeDef sConfig;

    // Compute the prescaler value to have TIM3 counter clock equal to 2 KHz
    uwPrescalerValue = (uint32_t)((SystemCoreClock / 2000) - 1);

    // Set TIM1 instance (Motion)
    TimCCHandle.Instance = TIM1;
    TimCCHandle.Init.Period = 65535;
    TimCCHandle.Init.Prescaler = uwPrescalerValue;
    TimCCHandle.Init.ClockDivision = 0;
    TimCCHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    if (HAL_TIM_OC_Init(&TimCCHandle) != HAL_OK)
    {
        STM32_Error_Handler();
    }

    // Configure the Output Compare channels
    // Common configuration for all channels
    sConfig.OCMode = TIM_OCMODE_TOGGLE;
    sConfig.OCPolarity = TIM_OCPOLARITY_LOW;

    // Output Compare Toggle Mode configuration: Channel1
    sConfig.Pulse = DEFAULT_TIM_CC1_PULSE;
    if (HAL_TIM_OC_ConfigChannel(&TimCCHandle, &sConfig, TIM_CHANNEL_1) != HAL_OK)
    {
        STM32_Error_Handler();
    }
}

/**
* @brief  Function for initializing the Real Time Clock
 * @param  None
 * @retval None
 */
static void InitRTC(void)
{
    // Configure RTC prescaler and RTC data registers */
    // RTC configured as follow:
    // - Hour Format    = Format 24
    // - Asynch Prediv  = Value according to source clock
    // - Synch Prediv   = Value according to source clock
    // - OutPut         = Output Disable
    // - OutPutPolarity = High Polarity
    // - OutPutType     = Open Drain

    RtcHandle.Instance = RTC;
    RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
    RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
    RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
    RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
    RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
    {
        STM32_Error_Handler();
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void STM32_Error_Handler(void)
{
    printf("FATAL: STM32 Error Handler\r\n");

    // User may add here some code to deal with this error
    while (1)
    {
    }
}

/**
 * @brief  Configures UART interface
 * @param  None
 * @retval None
 */
static void UART_Console_Init(void)
{
    UartHandle.Instance = USART1;
    UartHandle.Init.BaudRate = CFG_HW_UART1_BAUDRATE;
    UartHandle.Init.WordLength = CFG_HW_UART1_WORDLENGTH;
    UartHandle.Init.StopBits = CFG_HW_UART1_STOPBITS;
    UartHandle.Init.Parity = CFG_HW_UART1_PARITY;
    UartHandle.Init.Mode = CFG_HW_UART1_MODE;
    UartHandle.Init.HwFlowCtl = CFG_HW_UART1_HWFLOWCTL;
    UartHandle.AdvancedInit.AdvFeatureInit = CFG_HW_UART1_ADVFEATUREINIT;
    BSP_COM_Init(COM1, &UartHandle);
}

// EXTI line detection callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
        case USER_BUTTON_PIN:
            ButtonPressed = 1;
            break;
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
