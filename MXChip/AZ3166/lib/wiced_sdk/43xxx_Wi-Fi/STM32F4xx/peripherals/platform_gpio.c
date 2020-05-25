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
 * STM32F2xx common GPIO implementation
 */
#include "stdint.h"
#include "string.h"
#include "platform_peripheral.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "wwd_rtos_isr.h"
#include "wwd_assert.h"

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

/* Structure of runtime GPIO IRQ data */
typedef struct
{
    platform_gpio_port_t*        owner_port; // GPIO port owning the IRQ line (line is shared across all GPIO ports)
    platform_gpio_irq_callback_t handler;    // User callback
    void*                        arg;        // User argument to be passed to the callbackA
} platform_gpio_irq_data_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* GPIO peripheral clocks */
static const uint32_t gpio_peripheral_clocks[NUMBER_OF_GPIO_PORTS] =
{
    [0] = RCC_AHB1Periph_GPIOA,
    [1] = RCC_AHB1Periph_GPIOB,
    [2] = RCC_AHB1Periph_GPIOC,
    [3] = RCC_AHB1Periph_GPIOD,
    [4] = RCC_AHB1Periph_GPIOE,
    [5] = RCC_AHB1Periph_GPIOF,
    [6] = RCC_AHB1Periph_GPIOG,
    [7] = RCC_AHB1Periph_GPIOH,
};

/* Runtime GPIO IRQ data */
static volatile platform_gpio_irq_data_t gpio_irq_data[NUMBER_OF_GPIO_IRQ_LINES];

/******************************************************
 *            Platform Function Definitions
 ******************************************************/

platform_result_t platform_gpio_init( const platform_gpio_t* gpio, platform_pin_config_t config )
{
    GPIO_InitTypeDef  gpio_init_structure;
    uint8_t           port_number;

    wiced_assert( "bad argument", ( gpio != NULL ) );

    platform_mcu_powersave_disable();

    port_number = platform_gpio_get_port_number( gpio->port );

    /* Enable peripheral clock for this port */
    RCC->AHB1ENR |= gpio_peripheral_clocks[port_number];

    gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;
    gpio_init_structure.GPIO_Mode  = ( ( config == INPUT_PULL_UP ) || ( config == INPUT_PULL_DOWN ) || ( config == INPUT_HIGH_IMPEDANCE ) ) ? GPIO_Mode_IN : GPIO_Mode_OUT;
    gpio_init_structure.GPIO_OType = ( config == OUTPUT_PUSH_PULL ) ? GPIO_OType_PP : GPIO_OType_OD;

    if ( ( config == INPUT_PULL_UP ) || ( config == OUTPUT_OPEN_DRAIN_PULL_UP ) )
    {
        gpio_init_structure.GPIO_PuPd = GPIO_PuPd_UP;
    }
    else if ( config == INPUT_PULL_DOWN )
    {
        gpio_init_structure.GPIO_PuPd = GPIO_PuPd_DOWN;
    }
    else
    {
        gpio_init_structure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    }

    gpio_init_structure.GPIO_Pin = (uint32_t) ( 1 << gpio->pin_number );

    GPIO_Init( gpio->port, &gpio_init_structure );

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_deinit( const platform_gpio_t* gpio )
{
    GPIO_InitTypeDef  gpio_init_structure;

    wiced_assert( "bad argument", ( gpio != NULL ) );

    platform_mcu_powersave_disable();

    /* Set to Input high-impedance */
    gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;
    gpio_init_structure.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init_structure.GPIO_OType = GPIO_OType_PP; /* arbitrary. not applicable */
    gpio_init_structure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init_structure.GPIO_Pin   = (uint32_t) ( 1 << gpio->pin_number );

    GPIO_Init( gpio->port, &gpio_init_structure );

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_output_high( const platform_gpio_t* gpio )
{
    wiced_assert( "bad argument", ( gpio != NULL ) );

    platform_mcu_powersave_disable( );

    gpio->port->BSRRL = (uint16_t) ( 1 << gpio->pin_number );

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_output_low( const platform_gpio_t* gpio )
{
    wiced_assert( "bad argument", ( gpio != NULL ) );

    platform_mcu_powersave_disable();

    gpio->port->BSRRH = (uint16_t) ( 1 << gpio->pin_number );

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

wiced_bool_t platform_gpio_input_get( const platform_gpio_t* gpio )
{
    wiced_bool_t result;

    wiced_assert( "bad argument", ( gpio != NULL ) );

    platform_mcu_powersave_disable();

    result = ( ( gpio->port->IDR & (uint32_t) ( 1 << gpio->pin_number ) ) != 0 ) ? WICED_TRUE : WICED_FALSE;

    platform_mcu_powersave_enable();

    return result;
}

platform_result_t platform_gpio_irq_enable( const platform_gpio_t* gpio, platform_gpio_irq_trigger_t trigger, platform_gpio_irq_callback_t handler, void* arg )
{
    uint32_t            interrupt_line;
    EXTITrigger_TypeDef exti_trigger;

    wiced_assert( "bad argument", ( gpio != NULL ) && ( handler != NULL ) );

    interrupt_line = (uint32_t) ( 1 << gpio->pin_number );

    switch ( trigger )
    {
        case IRQ_TRIGGER_RISING_EDGE:
        {
            exti_trigger = EXTI_Trigger_Rising;
            break;
        }
        case IRQ_TRIGGER_FALLING_EDGE:
        {
            exti_trigger = EXTI_Trigger_Falling;
            break;
        }
        case IRQ_TRIGGER_BOTH_EDGES:
        {
            exti_trigger = EXTI_Trigger_Rising_Falling;
            break;
        }
        default:
        {
            return PLATFORM_BADARG;
        }
    }

    platform_mcu_powersave_disable();

    if ( ( EXTI->IMR & interrupt_line ) == 0 )
    {
        EXTI_InitTypeDef exti_init_structure;
        IRQn_Type        interrupt_vector = ( IRQn_Type ) 0;

        SYSCFG_EXTILineConfig( platform_gpio_get_port_number( gpio->port ), gpio->pin_number );

        exti_init_structure.EXTI_Trigger = exti_trigger;
        exti_init_structure.EXTI_Line    = interrupt_line;
        exti_init_structure.EXTI_Mode    = EXTI_Mode_Interrupt;
        exti_init_structure.EXTI_LineCmd = ENABLE;
        EXTI_Init( &exti_init_structure );

        if ( ( interrupt_line & 0x001F ) != 0 )
        {
            /* Line 0 to 4 */
            interrupt_vector = (IRQn_Type) ( EXTI0_IRQn + gpio->pin_number );
        }
        else if ( ( interrupt_line & 0x03E0 ) != 0 )
        {
            /* Line 5 to 9 */
            interrupt_vector = EXTI9_5_IRQn;
        }
        else if ( ( interrupt_line & 0xFC00 ) != 0 )
        {
            /* Line 10 to 15 */
            interrupt_vector = EXTI15_10_IRQn;
        }

        /* Must be lower priority than the value of configMAX_SYSCALL_INTERRUPT_PRIORITY otherwise FreeRTOS will not be able to mask the interrupt */
        NVIC_EnableIRQ( interrupt_vector );

        gpio_irq_data[gpio->pin_number].owner_port = gpio->port;
        gpio_irq_data[gpio->pin_number].handler    = handler;
        gpio_irq_data[gpio->pin_number].arg        = arg;

        platform_mcu_powersave_enable();

        return PLATFORM_SUCCESS;
    }

    platform_mcu_powersave_enable();

    return PLATFORM_NO_EFFECT;
}

platform_result_t platform_gpio_irq_disable( const platform_gpio_t* gpio )
{
    uint16_t interrupt_line;

    wiced_assert( "bad argument", ( gpio != NULL ) );

    interrupt_line = (uint16_t) ( 1 << gpio->pin_number );

    platform_mcu_powersave_disable();

    if ( ( EXTI->IMR & interrupt_line ) && gpio_irq_data[gpio->pin_number].owner_port == gpio->port )
    {
        wiced_bool_t     interrupt_line_used = WICED_FALSE;
        IRQn_Type        interrupt_vector    = ( IRQn_Type ) 0;
        EXTI_InitTypeDef exti_init_structure;

        /* Disable EXTI interrupt line */
        exti_init_structure.EXTI_Line    = (uint32_t)interrupt_line;
        exti_init_structure.EXTI_LineCmd = DISABLE;
        exti_init_structure.EXTI_Mode    = EXTI_Mode_Interrupt;
        exti_init_structure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_Init( &exti_init_structure );
        exti_init_structure.EXTI_Trigger = EXTI_Trigger_Falling;
        EXTI_Init( &exti_init_structure );

        /* Disable NVIC interrupt */
        if ( ( interrupt_line & 0x001F ) != 0 )
        {
            /* Line 0 to 4 */
            interrupt_vector = (IRQn_Type) ( EXTI0_IRQn + gpio->pin_number );
            interrupt_line_used = WICED_FALSE;
        }
        else if ( ( interrupt_line & 0x03E0 ) != 0 )
        {
            /* Line 5 to 9 */
            interrupt_vector = EXTI9_5_IRQn;
            interrupt_line_used = ( ( EXTI->IMR & 0x3e0U ) != 0 ) ? WICED_TRUE : WICED_FALSE;
        }
        else if ( ( interrupt_line & 0xFC00 ) != 0 )
        {
            /* Line 10 to 15 */
            interrupt_vector = EXTI15_10_IRQn;
            interrupt_line_used = ( ( EXTI->IMR & 0xfc00U ) != 0 ) ? WICED_TRUE : WICED_FALSE;
        }

        /* Some IRQ lines share a vector. Disable vector only if not used */
        if ( interrupt_line_used == WICED_FALSE )
        {
            NVIC_DisableIRQ( interrupt_vector );
        }

        gpio_irq_data[gpio->pin_number].owner_port = 0;
        gpio_irq_data[gpio->pin_number].handler    = 0;
        gpio_irq_data[gpio->pin_number].arg        = 0;

        platform_mcu_powersave_enable();

        return PLATFORM_SUCCESS;
    }

    platform_mcu_powersave_enable();

    return PLATFORM_NO_EFFECT;
}

platform_result_t platform_gpio_deepsleep_wakeup_enable( const platform_gpio_t* gpio, platform_gpio_irq_trigger_t trigger )
{
    return PLATFORM_UNSUPPORTED;
}

/******************************************************
 *      STM32F2xx Internal Function Definitions
 ******************************************************/

platform_result_t platform_gpio_irq_manager_init( void )
{
    memset( (void*)gpio_irq_data, 0, sizeof( gpio_irq_data ) );

    /* Switch on SYSCFG peripheral clock to allow writing into SYSCFG registers */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG, ENABLE );

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_set_alternate_function( platform_gpio_port_t* gpio_port, uint8_t pin_number, GPIOOType_TypeDef output_type, GPIOPuPd_TypeDef pull_up_down_type, uint8_t alternation_function )
{
    GPIO_InitTypeDef  gpio_init_structure;
    uint8_t           port_number = platform_gpio_get_port_number( gpio_port );

    platform_mcu_powersave_disable();

    /* Enable peripheral clock for this port */
    RCC->AHB1ENR |= gpio_peripheral_clocks[port_number];

    gpio_init_structure.GPIO_Speed = GPIO_Speed_25MHz;
    gpio_init_structure.GPIO_Mode  = GPIO_Mode_AF;
    gpio_init_structure.GPIO_OType = output_type;
    gpio_init_structure.GPIO_PuPd  = pull_up_down_type;
    gpio_init_structure.GPIO_Pin   = (uint32_t) ( 1 << pin_number );

    GPIO_Init( gpio_port, &gpio_init_structure );
    GPIO_PinAFConfig( gpio_port, pin_number, alternation_function );

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

uint8_t platform_gpio_get_port_number( platform_gpio_port_t* gpio_port )
{
    switch ( (uint32_t) gpio_port )
    {
        case GPIOA_BASE:
            return EXTI_PortSourceGPIOA;
        case GPIOB_BASE:
            return EXTI_PortSourceGPIOB;
        case GPIOC_BASE:
            return EXTI_PortSourceGPIOC;
        case GPIOD_BASE:
            return EXTI_PortSourceGPIOD;
        case GPIOE_BASE:
            return EXTI_PortSourceGPIOE;
        case GPIOF_BASE:
            return EXTI_PortSourceGPIOF;
        case GPIOG_BASE:
            return EXTI_PortSourceGPIOG;
        case GPIOH_BASE:
            return EXTI_PortSourceGPIOH;
        case GPIOI_BASE:
            return EXTI_PortSourceGPIOI;
        default:
            return INVALID_GPIO_PORT_NUMBER;
    }
}

/******************************************************
 *               IRQ Handler Definitions
 ******************************************************/

/* Common IRQ handler for all GPIOs */
WWD_RTOS_DEFINE_ISR( gpio_irq )
{
    uint32_t active_interrupt_vector = (uint32_t) ( ( SCB->ICSR & 0x3fU ) - 16 );
    uint32_t gpio_number;
    uint32_t interrupt_line;

    switch ( active_interrupt_vector )
    {
        case EXTI0_IRQn:
            interrupt_line = EXTI_Line0;
            gpio_number = 0;
            break;
        case EXTI1_IRQn:
            interrupt_line = EXTI_Line1;
            gpio_number = 1;
            break;
        case EXTI2_IRQn:
            interrupt_line = EXTI_Line2;
            gpio_number = 2;
            break;
        case EXTI3_IRQn:
            interrupt_line = EXTI_Line3;
            gpio_number = 3;
            break;
        case EXTI4_IRQn:
            interrupt_line = EXTI_Line4;
            gpio_number = 4;
            break;
        case EXTI9_5_IRQn:
            interrupt_line = EXTI_Line5;
            for ( gpio_number = 5; gpio_number < 10 && ( EXTI->PR & interrupt_line ) == 0; gpio_number++ )
            {
                interrupt_line <<= 1;
            }
            wiced_assert( "interrupt pin not found", gpio_number < 10 );
            break;
        case EXTI15_10_IRQn:
            interrupt_line = EXTI_Line10;
            for ( gpio_number = 10; gpio_number < 16 && ( EXTI->PR & interrupt_line ) == 0; gpio_number++ )
            {
                interrupt_line <<= 1;
            }
            wiced_assert( "interrupt pin not found", gpio_number < 16 );
            break;
        default:
            return;
    }

    /* Clear interrupt flag */
    EXTI->PR = interrupt_line;

    /* Call the respective GPIO interrupt handler/callback */
    if ( gpio_irq_data[gpio_number].handler != NULL )
    {
        void * arg = gpio_irq_data[gpio_number].arg; /* Avoids undefined order of access to volatiles */
        gpio_irq_data[gpio_number].handler( arg );
    }
}

/******************************************************
 *               IRQ Handler Mapping
 ******************************************************/

WWD_RTOS_MAP_ISR( gpio_irq , EXTI0_irq     )
WWD_RTOS_MAP_ISR( gpio_irq , EXTI1_irq     )
WWD_RTOS_MAP_ISR( gpio_irq , EXTI2_irq     )
WWD_RTOS_MAP_ISR( gpio_irq , EXTI3_irq     )
WWD_RTOS_MAP_ISR( gpio_irq , EXTI4_irq     )
WWD_RTOS_MAP_ISR( gpio_irq , EXTI9_5_irq   )
WWD_RTOS_MAP_ISR( gpio_irq , EXTI15_10_irq )
