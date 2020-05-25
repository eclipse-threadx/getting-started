#include <stdio.h>
#include "board_init.h"

#ifdef USE_COM_PORT
UART_HandleTypeDef UartHandle;
#endif

static void SystemClock_Config(void);
static void STM32_Error_Handler(void);
static void UART_Console_Init(void);

VOID board_init(void)
{

    GPIO_InitTypeDef gpio_init_structure;
    TIM_HandleTypeDef timer_handle;
    TIM_OC_InitTypeDef channel_config;

    /* Initialize STM32F412 HAL library.  */
    HAL_Init();

    /* Configure the system clock to 96 MHz.  */
    SystemClock_Config();

    // Initialize console
    UART_Console_Init();

    /* Enable GPIOA clock.  */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Enable GPIOB clock.  */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Enable GPIOC clock.  */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure pins for LEDs.  */
    gpio_init_structure.Pin = GPIO_PIN_2;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_init_structure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &gpio_init_structure);

    gpio_init_structure.Pin = GPIO_PIN_15;
    HAL_GPIO_Init(GPIOA, &gpio_init_structure);

    gpio_init_structure.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOC, &gpio_init_structure);

    /* Configure push buttons.  */
    gpio_init_structure.Pin = GPIO_PIN_4 | GPIO_PIN_10;
    gpio_init_structure.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_init_structure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio_init_structure);

    /* Enable interrupt.  */
    HAL_NVIC_SetPriority(EXTI4_IRQn, 0xE, 0xE);
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0xE, 0xE);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    /* Enable TIM2 clock.  */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* Enable TIM3 clock.  */
    __HAL_RCC_TIM3_CLK_ENABLE();

    timer_handle.Instance = TIM2;
    timer_handle.Init.Prescaler = 45;
    timer_handle.Init.Period = 2047;
    timer_handle.Init.ClockDivision = 0;
    timer_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    timer_handle.Init.RepetitionCounter = 0;
    timer_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&timer_handle);

    channel_config.OCMode = TIM_OCMODE_PWM1;
    channel_config.OCPolarity = TIM_OCPOLARITY_HIGH;
    channel_config.OCFastMode = TIM_OCFAST_DISABLE;
    channel_config.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    channel_config.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    channel_config.OCIdleState = TIM_OCIDLESTATE_RESET;
    channel_config.Pulse = 0;

    HAL_TIM_PWM_ConfigChannel(&timer_handle, &channel_config, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&timer_handle, TIM_CHANNEL_2);

    timer_handle.Instance = TIM3;
    HAL_TIM_PWM_Init(&timer_handle);
    HAL_TIM_PWM_ConfigChannel(&timer_handle, &channel_config, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&timer_handle, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&timer_handle, &channel_config, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&timer_handle, TIM_CHANNEL_2);

    /* Configure RGB LED pins.  */
    gpio_init_structure.Pin = GPIO_PIN_4;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &gpio_init_structure);
    gpio_init_structure.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOC, &gpio_init_structure);
    gpio_init_structure.Pin = GPIO_PIN_3;
    gpio_init_structure.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOB, &gpio_init_structure);
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 96000000
  *            HCLK(Hz)                       = 96000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 26000000
  *            PLL_M                          = 13
  *            PLL_N                          = 96
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 3
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 13;
    RCC_OscInitStruct.PLL.PLLN = 96;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    RCC_OscInitStruct.PLL.PLLR = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        STM32_Error_Handler();
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
    {
        STM32_Error_Handler();
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void STM32_Error_Handler(void)
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
    UartHandle.Instance = USART6;
    UartHandle.Init.BaudRate = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits = UART_STOPBITS_1;
    UartHandle.Init.Parity = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode = UART_MODE_TX_RX;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    // BSP_COM_Init(COM1, &UartHandle);
    HAL_UART_Init(&UartHandle);
}

#ifdef USE_COM_PORT
#if (defined(__GNUC__))
int _write(int fd, char * ptr, int len)
{
  HAL_UART_Transmit(&UartHandle, (uint8_t *) ptr, len, HAL_MAX_DELAY);
  return len;
}
#elif (defined(__ICCARM__))
size_t __write(int handle, const unsigned char *buffer, size_t size)
{

    /* Check for the command to flush all handles */
    if (handle == -1)
    {
        return 0;
    }
    /* Check for stdout and stderr      (only necessary if FILE descriptors are enabled.) */
    if (handle != 1 && handle != 2)
    {
        return -1;
    }

    if (HAL_UART_Transmit(&UartHandle, (uint8_t *)buffer, size, 5000) != HAL_OK)
    {
        return -1;
    }

    return size;
}

size_t __read(int handle, unsigned char *buffer, size_t size)
{

    /* Check for stdin      (only necessary if FILE descriptors are enabled) */
    if (handle != 0)
    {
        return -1;
    }

    if (HAL_UART_Receive(&UartHandle, (uint8_t *)buffer, size, 0x10000000) != HAL_OK)
    {
        return -1;
    }
    return size;
}
#elif defined (__CC_ARM)
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif
#endif

static int val;

__weak void button_a_callback()
{

    WIFI_LED_ON();
    AZURE_LED_ON();
    USER_LED_ON();
    if (BUTTON_A_IS_PRESSED)
    {
        val += 32;
        if (val > 2047)
            val = 2047;
        RGB_LED_SET_R(val);
        RGB_LED_SET_G(val);
        RGB_LED_SET_B(val);
    }
}

__weak void button_b_callback()
{

    WIFI_LED_OFF();
    AZURE_LED_OFF();
    USER_LED_OFF();
    if (BUTTON_B_IS_PRESSED)
    {
        val -= 32;
        if (val < 0)
            val = 0;
        RGB_LED_SET_R(val);
        RGB_LED_SET_G(val);
        RGB_LED_SET_B(val);
    }
}

void EXTI4_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
    case (BUTTON_A_PIN):

        button_a_callback();
        break;

    case (BUTTON_B_PIN):

        button_b_callback();
        break;

    default:
        break;
    }
}