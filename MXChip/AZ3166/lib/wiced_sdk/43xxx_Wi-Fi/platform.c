/*
 * Copyright 2019, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 * Defines board support package for BCM943362WCD6 board
 */
#include "platform.h"
#include "platform_config.h"
#include "platform_init.h"
#include "platform_isr.h"
#include "platform_peripheral.h"
#include "wwd_platform_common.h"
#include "wwd_rtos_isr.h"
#include "wiced_defaults.h"
#include "wiced_platform.h"
#include "platform_button.h"
#include "gpio_button.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* GPIO pin table. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_gpio_t platform_gpio_pins[] =
{
    [WICED_GPIO_1]  = { GPIOA,  0 },
    [WICED_GPIO_2]  = { GPIOA,  1 },
    [WICED_GPIO_3]  = { GPIOA,  2 },
    [WICED_GPIO_4]  = { GPIOA,  3 },
    [WICED_GPIO_5]  = { GPIOA,  4 },
    [WICED_GPIO_6]  = { GPIOA,  5 },
    [WICED_GPIO_7]  = { GPIOA,  6 },
    [WICED_GPIO_8]  = { GPIOA,  7 },
    [WICED_GPIO_9]  = { GPIOA,  9 },
    [WICED_GPIO_10] = { GPIOA, 10 },
    [WICED_GPIO_11] = { GPIOB,  6 },
    [WICED_GPIO_12] = { GPIOB,  7 },
    [WICED_GPIO_13] = { GPIOA, 13 },
    [WICED_GPIO_14] = { GPIOA, 14 },
    [WICED_GPIO_15] = { GPIOA, 15 },
    [WICED_GPIO_16] = { GPIOB,  3 },
    [WICED_GPIO_17] = { GPIOB,  4 },
};

/* ADC peripherals. Used WICED/platform/MCU/wiced_platform_common.c */
const platform_adc_t platform_adc_peripherals[] =
{
    [WICED_ADC_1] = {ADC1, ADC_Channel_1, RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_2]},
    [WICED_ADC_2] = {ADC1, ADC_Channel_2, RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_3]},
    [WICED_ADC_3] = {ADC1, ADC_Channel_3, RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_4]},
};

/* PWM peripherals. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_pwm_t platform_pwm_peripherals[] =
{
    [WICED_PWM_1]  = {TIM4, 1, RCC_APB1Periph_TIM4, GPIO_AF_TIM4, &platform_gpio_pins[WICED_GPIO_11]},
    [WICED_PWM_2]  = {TIM4, 2, RCC_APB1Periph_TIM4, GPIO_AF_TIM4, &platform_gpio_pins[WICED_GPIO_12]},
    [WICED_PWM_3]  = {TIM2, 2, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_2] }, /* or TIM5/Ch2                       */
    [WICED_PWM_4]  = {TIM2, 3, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_3] }, /* or TIM5/Ch3, TIM9/Ch1             */
    [WICED_PWM_5]  = {TIM2, 4, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_4] }, /* or TIM5/Ch4, TIM9/Ch2             */
    [WICED_PWM_6]  = {TIM2, 1, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_6] }, /* or TIM2_CH1_ETR, TIM8/Ch1N        */
    [WICED_PWM_7]  = {TIM3, 1, RCC_APB1Periph_TIM3, GPIO_AF_TIM3, &platform_gpio_pins[WICED_GPIO_7] }, /* or TIM1_BKIN, TIM8_BKIN, TIM13/Ch1*/
    [WICED_PWM_8]  = {TIM3, 2, RCC_APB1Periph_TIM3, GPIO_AF_TIM3, &platform_gpio_pins[WICED_GPIO_8] }, /* or TIM8/Ch1N, TIM14/Ch1           */
    [WICED_PWM_9]  = {TIM5, 2, RCC_APB1Periph_TIM5, GPIO_AF_TIM5, &platform_gpio_pins[WICED_GPIO_2] }, /* or TIM2/Ch2                       */
};

/* PWM peripherals. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_spi_t platform_spi_peripherals[] =
{
    [WICED_SPI_1]  =
    {
        .port                  = SPI1,
        .gpio_af               = GPIO_AF_SPI1,
        .peripheral_clock_reg  = RCC_APB2Periph_SPI1,
        .peripheral_clock_func = RCC_APB2PeriphClockCmd,
        .pin_mosi              = &platform_gpio_pins[WICED_GPIO_8],
        .pin_miso              = &platform_gpio_pins[WICED_GPIO_7],
        .pin_clock             = &platform_gpio_pins[WICED_GPIO_6],
        .tx_dma =
        {
            .controller        = DMA2,
            .stream            = DMA2_Stream5,
            .channel           = DMA_Channel_3,
            .irq_vector        = DMA2_Stream5_IRQn,
            .complete_flags    = DMA_HISR_TCIF5,
            .error_flags       = ( DMA_HISR_TEIF5 | DMA_HISR_FEIF5 | DMA_HISR_DMEIF5 ),
        },
        .rx_dma =
        {
            .controller        = DMA2,
            .stream            = DMA2_Stream0,
            .channel           = DMA_Channel_3,
            .irq_vector        = DMA2_Stream0_IRQn,
            .complete_flags    = DMA_LISR_TCIF0,
            .error_flags       = ( DMA_LISR_TEIF0 | DMA_LISR_FEIF0 | DMA_LISR_DMEIF0 ),
        },
    }
};

/* UART peripherals and runtime drivers. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_uart_t platform_uart_peripherals[] =
{
    [WICED_UART_1] =
    {
        .port               = USART1,
        .tx_pin             = &platform_gpio_pins[WICED_GPIO_9 ],
        .rx_pin             = &platform_gpio_pins[WICED_GPIO_10],
        .cts_pin            = NULL,
        .rts_pin            = NULL,
        .tx_dma_config =
        {
            .controller     = DMA2,
            .stream         = DMA2_Stream7,
            .channel        = DMA_Channel_4,
            .irq_vector     = DMA2_Stream7_IRQn,
            .complete_flags = DMA_HISR_TCIF7,
            .error_flags    = ( DMA_HISR_TEIF7 | DMA_HISR_FEIF7 ),
        },
        .rx_dma_config =
        {
            .controller     = DMA2,
            .stream         = DMA2_Stream2,
            .channel        = DMA_Channel_4,
            .irq_vector     = DMA2_Stream2_IRQn,
            .complete_flags = DMA_LISR_TCIF2,
            .error_flags    = ( DMA_LISR_TEIF2 | DMA_LISR_FEIF2 | DMA_LISR_DMEIF2 ),
        },
    },
    [WICED_UART_2] =
    {
        .port               = USART2,
        .tx_pin             = &platform_gpio_pins[WICED_GPIO_3],
        .rx_pin             = &platform_gpio_pins[WICED_GPIO_4],
        .cts_pin            = &platform_gpio_pins[WICED_GPIO_1],
        .rts_pin            = &platform_gpio_pins[WICED_GPIO_2],
        .tx_dma_config =
        {
            .controller     = DMA1,
            .stream         = DMA1_Stream6,
            .channel        = DMA_Channel_4,
            .irq_vector     = DMA1_Stream6_IRQn,
            .complete_flags = DMA_HISR_TCIF6,
            .error_flags    = ( DMA_HISR_TEIF6 | DMA_HISR_FEIF6 ),
        },
        .rx_dma_config =
        {
            .controller     = DMA1,
            .stream         = DMA1_Stream5,
            .channel        = DMA_Channel_4,
            .irq_vector     = DMA1_Stream5_IRQn,
            .complete_flags = DMA_HISR_TCIF5,
            .error_flags    = ( DMA_HISR_TEIF5 | DMA_HISR_FEIF5 | DMA_HISR_DMEIF5 ),
        },
    },
};
platform_uart_driver_t platform_uart_drivers[WICED_UART_MAX];

/* SPI flash. Exposed to the applications through include/wiced_platform.h */
#if defined ( WICED_PLATFORM_INCLUDES_SPI_FLASH )
const wiced_spi_device_t wiced_spi_flash =
{
    .port        = WICED_SPI_1,
    .chip_select = WICED_SPI_FLASH_CS,
    .speed       = 5000000,
    .mode        = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_NO_DMA | SPI_MSB_FIRST),
    .bits        = 8
};
#endif

/* UART standard I/O configuration */
#ifndef WICED_DISABLE_STDIO
static const platform_uart_config_t stdio_config =
{
    .baud_rate    = 115200,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
};
#endif

/* Wi-Fi control pins. Used by WICED/platform/MCU/wwd_platform_common.c */
const platform_gpio_t wifi_control_pins[] =
{
    [WWD_PIN_POWER      ] = { GPIOA,  8 },
    [WWD_PIN_RESET      ] = { GPIOA,  9 },
#if defined ( WICED_USE_WIFI_32K_CLOCK_MCO )
    [WWD_PIN_32K_CLK    ] = { GPIOA,  8 },
#else
    [WWD_PIN_32K_CLK        ] = { GPIOA, 11 },
#endif
    [WWD_PIN_BOOTSTRAP_0] = { GPIOB,  0 },
    [WWD_PIN_BOOTSTRAP_1] = { GPIOB,  1 },
};

/* Wi-Fi SDIO bus pins. Used by WICED/platform/STM32F2xx/WWD/wwd_SDIO.c */
const platform_gpio_t wifi_sdio_pins[] =
{
    [WWD_PIN_SDIO_OOB_IRQ] = { GPIOC,  0 },
    [WWD_PIN_SDIO_CLK    ] = { GPIOC, 12 },
    [WWD_PIN_SDIO_CMD    ] = { GPIOD,  2 },
    [WWD_PIN_SDIO_D0     ] = { GPIOC,  8 },
    [WWD_PIN_SDIO_D1     ] = { GPIOC,  9 },
    [WWD_PIN_SDIO_D2     ] = { GPIOC, 10 },
    [WWD_PIN_SDIO_D3     ] = { GPIOB,  5 },
};

/* Wi-Fi gSPI bus pins. Used by WICED/platform/STM32F2xx/WWD/wwd_SPI.c */
const platform_gpio_t wifi_spi_pins[] =
{
    [WWD_PIN_SPI_IRQ ] = { GPIOC,  9 },
    [WWD_PIN_SPI_CS  ] = { GPIOC, 11 },
    [WWD_PIN_SPI_CLK ] = { GPIOB, 13 },
    [WWD_PIN_SPI_MOSI] = { GPIOB, 15 },
    [WWD_PIN_SPI_MISO] = { GPIOB, 14 },
};

const gpio_button_t platform_gpio_buttons[PLATFORM_BUTTON_MAX] =
{
    [PLATFORM_BUTTON_1] =
    {
        .gpio       = WICED_BUTTON1,
        .config     = INPUT_PULL_UP,        /* no external pull ups on Cypress BCM9WCD1EVAL1 evaluation board */
        .polarity   = WICED_ACTIVE_LOW,
        .trigger    = IRQ_TRIGGER_BOTH_EDGES,
    },

    [PLATFORM_BUTTON_2] =
    {
        .gpio       = WICED_BUTTON2,
        .config     = INPUT_PULL_UP,        /* no external pull ups on Cypress BCM9WCD1EVAL1 evaluation board */
        .polarity   = WICED_ACTIVE_LOW,
        .trigger    = IRQ_TRIGGER_BOTH_EDGES,
    },
};

const wiced_gpio_t platform_gpio_leds[PLATFORM_LED_COUNT] =
{
     [WICED_LED_INDEX_1] = WICED_LED1,
     [WICED_LED_INDEX_2] = WICED_LED2,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

void platform_init_peripheral_irq_priorities( void )
{
    /* Interrupt priority setup. Called by WICED/platform/MCU/STM32F2xx/platform_init.c */
    NVIC_SetPriority( RTC_WKUP_IRQn    ,  1 ); /* RTC Wake-up event   */
    NVIC_SetPriority( SDIO_IRQn        ,  2 ); /* WLAN SDIO           */
    NVIC_SetPriority( DMA2_Stream3_IRQn,  3 ); /* WLAN SDIO DMA       */
    NVIC_SetPriority( DMA1_Stream3_IRQn,  3 ); /* WLAN SPI DMA        */
    NVIC_SetPriority( USART1_IRQn      ,  6 ); /* WICED_UART_1        */
    NVIC_SetPriority( USART2_IRQn      ,  6 ); /* WICED_UART_2        */
    NVIC_SetPriority( DMA2_Stream7_IRQn,  7 ); /* WICED_UART_1 TX DMA */
    NVIC_SetPriority( DMA2_Stream2_IRQn,  7 ); /* WICED_UART_1 RX DMA */
    NVIC_SetPriority( DMA1_Stream6_IRQn,  7 ); /* WICED_UART_2 TX DMA */
    NVIC_SetPriority( DMA1_Stream5_IRQn,  7 ); /* WICED_UART_2 RX DMA */
    NVIC_SetPriority( EXTI0_IRQn       , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI1_IRQn       , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI2_IRQn       , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI3_IRQn       , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI4_IRQn       , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI9_5_IRQn     , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI15_10_IRQn   , 14 ); /* GPIO                */
}

/* LEDs on this platform are active HIGH */
platform_result_t platform_led_set_state(int led_index, int off_on )
{
    if ((led_index >= 0) && (led_index < PLATFORM_LED_COUNT))
    {
        switch (off_on)
        {
            case WICED_LED_OFF:
                platform_gpio_output_low( &platform_gpio_pins[platform_gpio_leds[led_index]] );
                break;
            case WICED_LED_ON:
                platform_gpio_output_high( &platform_gpio_pins[platform_gpio_leds[led_index]] );
                break;
        }
        return PLATFORM_SUCCESS;
    }
    return PLATFORM_BADARG;
}

void platform_led_init( void )
{
    /* Initialise LEDs and turn off by default */
    platform_gpio_init( &platform_gpio_pins[WICED_LED1], OUTPUT_PUSH_PULL );
    platform_gpio_init( &platform_gpio_pins[WICED_LED2], OUTPUT_PUSH_PULL );
    platform_led_set_state(WICED_LED_INDEX_1, WICED_LED_ON);
    platform_led_set_state(WICED_LED_INDEX_2, WICED_LED_OFF);
 }


void platform_init_external_devices( void )
{
    /* Initialize LEDs and turn off by default */
    platform_led_init();

    /* Initialize buttons */
    platform_gpio_init( &platform_gpio_pins[ platform_gpio_buttons[PLATFORM_BUTTON_1].gpio ], platform_gpio_buttons[PLATFORM_BUTTON_1].config );
    platform_gpio_init( &platform_gpio_pins[ platform_gpio_buttons[PLATFORM_BUTTON_2].gpio ], platform_gpio_buttons[PLATFORM_BUTTON_2].config );

#ifndef WICED_DISABLE_STDIO
    /* Initialise UART standard I/O */
    platform_stdio_init( &platform_uart_drivers[STDIO_UART], &platform_uart_peripherals[STDIO_UART], &stdio_config );
#endif
}

uint32_t  platform_get_button_press_time ( int button_index, int led_index, uint32_t max_time )
{
    int             button_gpio;
    uint32_t        button_press_timer = 0;
    int             led_state = 0;

    /* Initialize input */
    button_gpio     = platform_gpio_buttons[button_index].gpio;
    platform_gpio_init( &platform_gpio_pins[ button_gpio ], platform_gpio_buttons[button_index].config );

    while ( (PLATFORM_BUTTON_PRESSED_STATE == platform_gpio_input_get(&platform_gpio_pins[ button_gpio ])) )
    {
        /* wait a bit */
        host_rtos_delay_milliseconds( PLATFORM_BUTTON_PRESS_CHECK_PERIOD );

        /* Toggle LED */
        platform_led_set_state(led_index, (led_state == 0) ? WICED_LED_OFF : WICED_LED_ON);
        led_state ^= 0x01;

        /* keep track of time */
        button_press_timer += PLATFORM_BUTTON_PRESS_CHECK_PERIOD;
        if ((max_time > 0) && (button_press_timer >= max_time))
        {
            break;
        }
    }

     /* turn off the LED */
    platform_led_set_state(led_index, WICED_LED_OFF );

    /* deinit the button */
    platform_gpio_deinit( &platform_gpio_pins[ button_gpio ] );

    return button_press_timer;
}

uint32_t  platform_get_factory_reset_button_time ( uint32_t max_time )
{
    return platform_get_button_press_time ( PLATFORM_FACTORY_RESET_BUTTON_INDEX, PLATFORM_RED_LED_INDEX, max_time );
}


/******************************************************
 *           Interrupt Handler Definitions
 ******************************************************/
#if 0
WWD_RTOS_DEFINE_ISR( usart1_irq )
{
    platform_uart_irq( &platform_uart_drivers[WICED_UART_1] );
}

WWD_RTOS_DEFINE_ISR( usart2_irq )
{
    platform_uart_irq( &platform_uart_drivers[WICED_UART_2] );
}

WWD_RTOS_DEFINE_ISR( usart1_tx_dma_irq )
{
    platform_uart_tx_dma_irq( &platform_uart_drivers[WICED_UART_1] );
}

WWD_RTOS_DEFINE_ISR( usart2_tx_dma_irq )
{
    platform_uart_tx_dma_irq( &platform_uart_drivers[WICED_UART_2] );
}

WWD_RTOS_DEFINE_ISR( usart1_rx_dma_irq )
{
    platform_uart_rx_dma_irq( &platform_uart_drivers[WICED_UART_1] );
}

WWD_RTOS_DEFINE_ISR( usart2_rx_dma_irq )
{
    platform_uart_rx_dma_irq( &platform_uart_drivers[WICED_UART_2] );
}

/******************************************************
 *            Interrupt Handlers Mapping
 ******************************************************/

/* These DMA assignments can be found STM32F2xx datasheet DMA section */
WWD_RTOS_MAP_ISR( usart1_irq       , USART1_irq       )
WWD_RTOS_MAP_ISR( usart1_tx_dma_irq, DMA2_Stream7_irq )
WWD_RTOS_MAP_ISR( usart1_rx_dma_irq, DMA2_Stream2_irq )
WWD_RTOS_MAP_ISR( usart2_irq       , USART2_irq       )
WWD_RTOS_MAP_ISR( usart2_tx_dma_irq, DMA1_Stream6_irq )
WWD_RTOS_MAP_ISR( usart2_rx_dma_irq, DMA1_Stream5_irq )
#endif