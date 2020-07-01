#include <stdio.h>
#include "board_init.h"
#include "sensor.h"

/* I2C handler declaration */
I2C_HandleTypeDef I2cHandle;

#define I2C_ADDRESS        0x30F

/* I2C SPEEDCLOCK define to max value: 400 KHz on STM32F4xx*/
#define I2C_SPEEDCLOCK   400000
#define I2C_DUTYCYCLE    I2C_DUTYCYCLE_2

/* Definition for I2Cx clock resources */
#define I2Cx                             I2C1

#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_8
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SDA_PIN                    GPIO_PIN_9
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SCL_SDA_AF                 GPIO_AF4_I2C1

UART_HandleTypeDef UartHandle;


void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  I2Cx_SCL_GPIO_CLK_ENABLE();
  I2Cx_SDA_GPIO_CLK_ENABLE();
  /* Enable I2Cx clock */
  I2Cx_CLK_ENABLE(); 

  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* I2C TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);
    
  /* I2C RX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;
  GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);
}

static void SystemClock_Config(void);
static void STM32_Error_Handler(void);
static void UART_Console_Init(void);


static void Init_MEM1_Sensors(void)
{    
    if(SENSOR_OK != lps22hb_config())
    {
        printf("Init Error Pressure Sensor\r\n");
    }
    if(SENSOR_OK != hts221_config())
    {
        printf("Init Error Humidity-Temperature Sensor\r\n");
    }    
    printf("Humidity-Temperatur Sensor OK \r\n");
    if(SENSOR_OK != lsm6dsl_config())
    {
        printf("Init Error Accelerometer Sensor\r\n");
    }
    if(SENSOR_OK != lis2mdl_config())
    {
        printf("Init Error Magnetometer Sensor\r\n");
    }
}

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
    
    
    I2cHandle.Instance             = I2Cx;
    I2cHandle.Init.ClockSpeed      = I2C_SPEEDCLOCK;
    I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE;
    I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
    I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_10BIT;
    I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2cHandle.Init.OwnAddress2     = 0xFF;
    I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;  

    if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
    {
    /* Initialization Error */
    while(1);    
    }
    
    Init_MEM1_Sensors();
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