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
 * STM32F2xx UART implementation
 */
#include <stdint.h>
#include <string.h>
#include "platform_config.h"
#include "platform_peripheral.h"
#include "platform_sleep.h"
#include "platform_assert.h"
#include "wwd_assert.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define DMA_INTERRUPT_FLAGS  ( DMA_IT_TC | DMA_IT_TE | DMA_IT_DME | DMA_IT_FE )

#define UART_MAX_TRANSMIT_WAIT_TIME_MS  (10)

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

static platform_result_t receive_bytes       ( platform_uart_driver_t* driver, void* data, uint32_t size, uint32_t timeout );
static uint32_t          get_dma_irq_status  ( DMA_Stream_TypeDef* stream );
static void              clear_dma_interrupts( DMA_Stream_TypeDef* stream, uint32_t flags );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* UART alternate functions */
static const uint8_t uart_alternate_functions[NUMBER_OF_UART_PORTS] =
{
    [0] = GPIO_AF_USART1,
    [1] = GPIO_AF_USART2,
#if defined(STM32F401xx) || defined(STM32F411xE) || defined(STM32F412xG)
    [2] = GPIO_AF_USART6,
#if defined(STM32F412xG)
    [3] = GPIO_AF_USART3,
#endif
#else
    [2] = GPIO_AF_USART3,
    [3] = GPIO_AF_UART4,
    [4] = GPIO_AF_UART5,
    [5] = GPIO_AF_USART6,
#if defined(STM32F427_437xx) || defined(STM32F429_439xx)
    [6] = GPIO_AF_UART7,
    [7] = GPIO_AF_UART8,
#endif
#endif
};

/* UART peripheral clock functions */
static const platform_peripheral_clock_function_t uart_peripheral_clock_functions[NUMBER_OF_UART_PORTS] =
{
    [0] = RCC_APB2PeriphClockCmd,
    [1] = RCC_APB1PeriphClockCmd,
#if defined(STM32F401xx) || defined(STM32F411xE) || defined(STM32F412xG)
    [2] = RCC_APB2PeriphClockCmd,
#if defined(STM32F412xG)
    [3] = RCC_APB1PeriphClockCmd,
#endif
#else
    [2] = RCC_APB1PeriphClockCmd,
    [3] = RCC_APB1PeriphClockCmd,
    [4] = RCC_APB1PeriphClockCmd,
    [5] = RCC_APB2PeriphClockCmd,
#if defined(STM32F427_437xx) || defined(STM32F429_439xx)
    [6] = RCC_APB1PeriphClockCmd,
    [7] = RCC_APB1PeriphClockCmd,
#endif
#endif
};

/* UART peripheral clocks */
static const uint32_t uart_peripheral_clocks[NUMBER_OF_UART_PORTS] =
{
    [0] = RCC_APB2Periph_USART1,
    [1] = RCC_APB1Periph_USART2,
#if defined(STM32F401xx) || defined(STM32F411xE) || defined(STM32F412xG)
    [2] = RCC_APB2Periph_USART6,
#if defined(STM32F412xG)
    [3] = RCC_APB1Periph_USART3,
#endif
#else
    [2] = RCC_APB1Periph_USART3,
    [3] = RCC_APB1Periph_UART4,
    [4] = RCC_APB1Periph_UART5,
    [5] = RCC_APB2Periph_USART6,
#if defined(STM32F427_437xx) || defined(STM32F429_439xx)
    [6] = RCC_APB1Periph_UART7,
    [7] = RCC_APB1Periph_UART8,
#endif
#endif
};

/* UART interrupt vectors */
static const IRQn_Type uart_irq_vectors[NUMBER_OF_UART_PORTS] =
{
    [0] = USART1_IRQn,
    [1] = USART2_IRQn,
#if defined(STM32F401xx) || defined(STM32F411xE)|| defined(STM32F412xG)
    [2] = USART6_IRQn,
#if defined(STM32F412xG)
    [3] = USART3_IRQn,
#endif
#else
    [2] = USART3_IRQn,
    [3] = UART4_IRQn,
    [4] = UART5_IRQn,
    [5] = USART6_IRQn,
#if defined(STM32F427_437xx) || defined(STM32F429_439xx)
    [6] = UART7_IRQn,
    [7] = UART8_IRQn,
#endif
#endif
};

/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_uart_init( platform_uart_driver_t* driver, const platform_uart_t* peripheral, const platform_uart_config_t* config, wiced_ring_buffer_t* optional_ring_buffer )
{
    DMA_InitTypeDef   dma_init_structure;
    USART_InitTypeDef uart_init_structure;
    uint32_t          uart_number;

    wiced_assert( "bad argument", ( driver != NULL ) && ( peripheral != NULL ) && ( config != NULL ) );
    wiced_assert( "Bad ring buffer", (optional_ring_buffer == NULL) || ((optional_ring_buffer->buffer != NULL ) && (optional_ring_buffer->size != 0)) );

    platform_mcu_powersave_disable();

    uart_number = platform_uart_get_port_number( peripheral->port );

    driver->rx_size              = 0;
    driver->tx_size              = 0;
    driver->last_transmit_result = PLATFORM_SUCCESS;
    driver->last_receive_result  = PLATFORM_SUCCESS;
    driver->peripheral           = (platform_uart_t*)peripheral;
    host_rtos_init_semaphore( &driver->tx_complete );
    host_rtos_init_semaphore( &driver->rx_complete );

    /* Configure TX and RX pin_mapping */
    platform_gpio_set_alternate_function( peripheral->tx_pin->port, peripheral->tx_pin->pin_number, GPIO_OType_PP, GPIO_PuPd_NOPULL, uart_alternate_functions[ uart_number ] );
    platform_gpio_set_alternate_function( peripheral->rx_pin->port, peripheral->rx_pin->pin_number, GPIO_OType_PP, GPIO_PuPd_NOPULL, uart_alternate_functions[ uart_number ] );

    if ( ( peripheral->cts_pin != NULL ) && ( config->flow_control == FLOW_CONTROL_CTS || config->flow_control == FLOW_CONTROL_CTS_RTS ) )
    {
        platform_gpio_set_alternate_function( peripheral->cts_pin->port, peripheral->cts_pin->pin_number, GPIO_OType_PP, GPIO_PuPd_NOPULL, uart_alternate_functions[ uart_number ] );
    }

    if ( ( peripheral->rts_pin != NULL ) && ( config->flow_control == FLOW_CONTROL_RTS || config->flow_control == FLOW_CONTROL_CTS_RTS ) )
    {
        platform_gpio_set_alternate_function( peripheral->rts_pin->port, peripheral->rts_pin->pin_number, GPIO_OType_PP, GPIO_PuPd_NOPULL, uart_alternate_functions[ uart_number ] );
    }

    /* Enable UART peripheral clock */
    uart_peripheral_clock_functions[ uart_number ]( uart_peripheral_clocks[ uart_number ], ENABLE );

    uart_init_structure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;
    uart_init_structure.USART_BaudRate   = config->baud_rate;
    uart_init_structure.USART_WordLength = ( ( config->data_width == DATA_WIDTH_9BIT ) || ( ( config->data_width == DATA_WIDTH_8BIT ) && ( config->parity != NO_PARITY ) ) ) ? USART_WordLength_9b : USART_WordLength_8b;
    uart_init_structure.USART_StopBits   = ( config->stop_bits == STOP_BITS_1 ) ? USART_StopBits_1 : USART_StopBits_2;

    switch ( config->parity )
    {
        case NO_PARITY:
            uart_init_structure.USART_Parity = USART_Parity_No;
            break;

        case EVEN_PARITY:
            uart_init_structure.USART_Parity = USART_Parity_Even;
            break;

        case ODD_PARITY:
            uart_init_structure.USART_Parity = USART_Parity_Odd;
            break;

        default:
            return PLATFORM_BADARG;
    }

    switch ( config->flow_control )
    {
        case FLOW_CONTROL_DISABLED:
            uart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
            break;

        case FLOW_CONTROL_CTS:
            uart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_CTS;
            break;

        case FLOW_CONTROL_RTS:
            uart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS;
            break;

        case FLOW_CONTROL_CTS_RTS:
            uart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
            break;

        default:
            return PLATFORM_BADARG;
    }


    /* Initialise USART peripheral */
    USART_DeInit( peripheral->port );
    USART_Init( peripheral->port, &uart_init_structure );

    /**************************************************************************
     * Initialise STM32 DMA registers
     * Note: If DMA is used, USART interrupt isn't enabled.
     **************************************************************************/

    /* Enable DMA peripheral clock */
    if ( peripheral->tx_dma_config.controller == DMA1 )
    {
        RCC->AHB1ENR |= RCC_AHB1Periph_DMA1;
    }
    else
    {
        RCC->AHB1ENR |= RCC_AHB1Periph_DMA2;
    }

    /* Fill init structure with common DMA settings */
    dma_init_structure.DMA_PeripheralInc   = DMA_PeripheralInc_Disable;
    dma_init_structure.DMA_MemoryInc       = DMA_MemoryInc_Enable;
    dma_init_structure.DMA_Priority        = DMA_Priority_VeryHigh;
    dma_init_structure.DMA_FIFOMode        = DMA_FIFOMode_Disable;
    dma_init_structure.DMA_FIFOThreshold   = DMA_FIFOThreshold_Full;
    dma_init_structure.DMA_MemoryBurst     = DMA_MemoryBurst_Single;
    dma_init_structure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    if ( config->data_width == DATA_WIDTH_9BIT )
    {
        dma_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
        dma_init_structure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    }
    else
    {
        dma_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        dma_init_structure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    }

    /* Initialise TX DMA */
    DMA_DeInit( peripheral->tx_dma_config.stream );
    dma_init_structure.DMA_Channel            = peripheral->tx_dma_config.channel;
    dma_init_structure.DMA_PeripheralBaseAddr = (uint32_t) &peripheral->port->DR;
    dma_init_structure.DMA_Memory0BaseAddr    = (uint32_t) 0;
    dma_init_structure.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
    dma_init_structure.DMA_BufferSize         = 0xFFFF;                     // This parameter will be configured during communication
    dma_init_structure.DMA_Mode               = DMA_Mode_Normal;
    DMA_Init( peripheral->tx_dma_config.stream, &dma_init_structure );

    /* Initialise RX DMA */
    DMA_DeInit( peripheral->rx_dma_config.stream );
    dma_init_structure.DMA_Channel            = peripheral->rx_dma_config.channel;
    dma_init_structure.DMA_PeripheralBaseAddr = (uint32_t) &peripheral->port->DR;
    dma_init_structure.DMA_Memory0BaseAddr    = 0;
    dma_init_structure.DMA_DIR                = DMA_DIR_PeripheralToMemory;
    dma_init_structure.DMA_BufferSize         = 0xFFFF;                     // This parameter will be configured during communication
    dma_init_structure.DMA_Mode               = DMA_Mode_Normal;
    DMA_Init( peripheral->rx_dma_config.stream, &dma_init_structure );

    /**************************************************************************
     * Initialise STM32 DMA interrupts
     **************************************************************************/

    /* Configure TX DMA interrupt on Cortex-M3 */
    NVIC_EnableIRQ( peripheral->tx_dma_config.irq_vector );

    /* Enable TC (transfer complete) and TE (transfer error) interrupts on source */
    clear_dma_interrupts( peripheral->tx_dma_config.stream, peripheral->tx_dma_config.complete_flags | peripheral->tx_dma_config.error_flags );
    DMA_ITConfig( peripheral->tx_dma_config.stream, DMA_INTERRUPT_FLAGS, ENABLE );

    /* Enable USART interrupt vector in Cortex-M3 */
    NVIC_EnableIRQ( uart_irq_vectors[uart_number] );

    /* Enable USART */
    USART_Cmd( peripheral->port, ENABLE );

    /* Enable both transmit and receive */
    peripheral->port->CR1 |= USART_CR1_TE;
    peripheral->port->CR1 |= USART_CR1_RE;

    /* Setup ring buffer */
    if ( optional_ring_buffer != NULL )
    {
        /* Note that the ring_buffer should've been initialised first */
        driver->rx_buffer = optional_ring_buffer;
        driver->rx_size   = 0;
        receive_bytes( driver, optional_ring_buffer->buffer, optional_ring_buffer->size, 0 );
    }
    else
    {
        /* Not using ring buffer. Configure RX DMA interrupt on Cortex-M3 */
        NVIC_EnableIRQ( peripheral->rx_dma_config.irq_vector );

        /* Enable TC (transfer complete) and TE (transfer error) interrupts on source */
        clear_dma_interrupts( peripheral->rx_dma_config.stream, peripheral->rx_dma_config.complete_flags | peripheral->rx_dma_config.error_flags );
        DMA_ITConfig( peripheral->rx_dma_config.stream, DMA_INTERRUPT_FLAGS, ENABLE );
    }

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_uart_deinit( platform_uart_driver_t* driver )
{
    uint8_t          uart_number;

    wiced_assert( "bad argument", ( driver != NULL ) );

    platform_mcu_powersave_disable();

    uart_number = platform_uart_get_port_number( driver->peripheral->port );

    /* Disable USART */
    USART_Cmd( driver->peripheral->port, DISABLE );

    /* Deinitialise USART */
    USART_DeInit( driver->peripheral->port );

    /**************************************************************************
     * De-initialise STM32 DMA and interrupt
     **************************************************************************/

    /* Deinitialise DMA streams */
    DMA_DeInit( driver->peripheral->tx_dma_config.stream );
    DMA_DeInit( driver->peripheral->rx_dma_config.stream );

    /* Disable TC (transfer complete) interrupt at the source */
    DMA_ITConfig( driver->peripheral->tx_dma_config.stream, DMA_INTERRUPT_FLAGS, DISABLE );
    DMA_ITConfig( driver->peripheral->rx_dma_config.stream, DMA_INTERRUPT_FLAGS, DISABLE );

    /* Disable transmit DMA interrupt at Cortex-M3 */
    NVIC_DisableIRQ( driver->peripheral->tx_dma_config.irq_vector );

    /**************************************************************************
     * De-initialise STM32 USART interrupt
     **************************************************************************/

    USART_ITConfig( driver->peripheral->port, USART_IT_RXNE, DISABLE );

    /* Disable UART interrupt vector on Cortex-M3 */
    NVIC_DisableIRQ( driver->peripheral->rx_dma_config.irq_vector );

    /* Disable registers clocks */
    uart_peripheral_clock_functions[uart_number]( uart_peripheral_clocks[uart_number], DISABLE );

    host_rtos_deinit_semaphore( &driver->rx_complete );
    host_rtos_deinit_semaphore( &driver->tx_complete );
    driver->rx_size              = 0;
    driver->tx_size              = 0;
    driver->last_transmit_result = PLATFORM_SUCCESS;
    driver->last_receive_result  = PLATFORM_SUCCESS;

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_uart_transmit_bytes( platform_uart_driver_t* driver, const uint8_t* data_out, uint32_t size )
{
    wiced_assert( "bad argument", ( driver != NULL ) && ( data_out != NULL ) && ( size != 0 ) );

    platform_mcu_powersave_disable();

    /* Clear interrupt status before enabling DMA otherwise error occurs immediately */
    clear_dma_interrupts( driver->peripheral->tx_dma_config.stream, driver->peripheral->tx_dma_config.complete_flags | driver->peripheral->tx_dma_config.error_flags );

    /* Init DMA parameters and variables */
    driver->last_transmit_result                    = PLATFORM_SUCCESS;
    driver->tx_size                                 = size;
    driver->peripheral->tx_dma_config.stream->CR   &= ~(uint32_t) DMA_SxCR_CIRC;
    driver->peripheral->tx_dma_config.stream->NDTR  = size;
    driver->peripheral->tx_dma_config.stream->M0AR  = (uint32_t)data_out;
    driver->peripheral->tx_dma_config.stream->CR   |= DMA_SxCR_EN;
    USART_DMACmd( driver->peripheral->port, USART_DMAReq_Tx, ENABLE );

    /* Wait for transmission complete */
    host_rtos_get_semaphore( &driver->tx_complete, NEVER_TIMEOUT, WICED_TRUE );
    while ( ( driver->peripheral->port->SR & USART_SR_TC ) == 0 )
    {
    }

    /* Disable DMA and clean up */
    USART_DMACmd( driver->peripheral->port, USART_DMAReq_Tx, DISABLE );
    driver->tx_size = 0;

    platform_mcu_powersave_enable();

    return driver->last_transmit_result;
}

platform_result_t platform_uart_exception_transmit_bytes( platform_uart_driver_t* driver, const uint8_t* data_out, uint32_t size )
{
    /* Called in exception context and must not use interrupts. */

    wiced_assert( "bad argument", ( driver != NULL ) && ( data_out != NULL ) && ( size != 0 ) );

    do
    {
        wwd_time_t   start_time   = host_rtos_get_time();
        wwd_time_t   elapsed_time = 0;

        /* Wait for transmit data register empty */
        while ( ((driver->peripheral->port->SR & USART_SR_TXE) == 0) && (elapsed_time < UART_MAX_TRANSMIT_WAIT_TIME_MS) )
        {
            elapsed_time = host_rtos_get_time( ) - start_time;
        }

        if ( (driver->peripheral->port->SR & USART_SR_TXE) == 0 )
        {
            return PLATFORM_TIMEOUT;
        }

        driver->peripheral->port->DR = *data_out++;
        size--;
    }
    while ( size != 0 );

    return PLATFORM_SUCCESS;
}

platform_result_t platform_uart_receive_bytes( platform_uart_driver_t* driver, uint8_t* data_in, uint32_t* expected_data_size, uint32_t timeout_ms )
{
    platform_result_t result = PLATFORM_SUCCESS;
    uint32_t bytes_left      = 0;

    wiced_assert( "bad argument", ( driver != NULL ) && ( data_in != NULL ) && ( expected_data_size != NULL ) && ( *expected_data_size != 0 ) );

    bytes_left = *expected_data_size;

    if ( driver->rx_buffer != NULL )
    {
        while ( bytes_left != 0 )
        {
            uint32_t transfer_size = MIN( driver->rx_buffer->size / 2, bytes_left );

            /* Check if ring buffer already contains the required amount of data. */
            if ( transfer_size > ring_buffer_used_space( driver->rx_buffer ) )
            {
                wwd_result_t wwd_result;

                /* Set rx_size and wait in rx_complete semaphore until data reaches rx_size or timeout occurs */
                driver->last_receive_result = PLATFORM_SUCCESS;
                driver->rx_size             = transfer_size;

                wwd_result = host_rtos_get_semaphore( &driver->rx_complete, timeout_ms, WICED_TRUE );
                /* Reset rx_size to prevent semaphore being set while nothing waits for the data */
                driver->rx_size = 0;

                if ( wwd_result == WWD_TIMEOUT )
                {
                    /* Semaphore timeout. breaks from the while loop */
                    result = PLATFORM_TIMEOUT;
                    break;
                }
                else
                {
                    /* No timeout. retrieve result */
                    result = driver->last_receive_result;
                }
            }

            bytes_left -= transfer_size;

            // Grab data from the buffer
            do
            {
                uint8_t* available_data;
                uint32_t bytes_available;

                ring_buffer_get_data( driver->rx_buffer, &available_data, &bytes_available );
                bytes_available = MIN( bytes_available, transfer_size );
                memcpy( data_in, available_data, bytes_available );
                transfer_size -= bytes_available;
                data_in = ( (uint8_t*) data_in + bytes_available );
                ring_buffer_consume( driver->rx_buffer, bytes_available );
            } while ( transfer_size != 0 );
        }

        /* update actual amount of data rx */
        *expected_data_size -= bytes_left;
    }
    else
    {
        /* Note: this code assumes DMA doesn't run into errors.  Check driver->last_receive_result for DMA status. */
        result = receive_bytes( driver, data_in, *expected_data_size, timeout_ms );

        /* nothing received if we time out */
        if ( PLATFORM_TIMEOUT == result )
        {
            *expected_data_size = 0;
        }
    }

    return result;
}

uint8_t platform_uart_get_port_number( USART_TypeDef* uart )
{
    if ( uart == USART1 )
    {
        return 0;
    }
    else if ( uart == USART2 )
    {
        return 1;
    }
#if defined(STM32F401xx) || defined(STM32F411xE) || defined(STM32F412xG)
    else if ( uart == USART6 )
    {
        return 2;
    }
#if defined(STM32F412xG)
    else if ( uart == USART3 )
    {
        return 3;
    }
#endif
#else
    else if ( uart == USART3 )
    {
        return 2;
    }
    else if ( uart == UART4 )
    {
        return 3;
    }
    else if ( uart == UART5 )
    {
        return 4;
    }
    else if ( uart == USART6 )
    {
        return 5;
    }
#if defined(STM32F427_437xx) || defined(STM32F429_439xx)
    else if (uart == UART7)
    {
        return 6;
    }
    else if (uart == UART8)
    {
        return 7;
    }
#endif
#endif
    else
    {
        return INVALID_UART_PORT_NUMBER;
    }
}

static platform_result_t receive_bytes( platform_uart_driver_t* driver, void* data, uint32_t size, uint32_t timeout )
{
    platform_result_t result = PLATFORM_SUCCESS;

    if ( driver->rx_buffer != NULL )
    {
        driver->peripheral->rx_dma_config.stream->CR |= DMA_SxCR_CIRC;

        // Enabled individual byte interrupts so progress can be updated
        USART_ClearITPendingBit( driver->peripheral->port, USART_IT_RXNE );
        USART_ITConfig( driver->peripheral->port, USART_IT_RXNE, ENABLE );
    }
    else
    {
        driver->rx_size = size;
        driver->peripheral->rx_dma_config.stream->CR &= ~(uint32_t) DMA_SxCR_CIRC;
    }

    clear_dma_interrupts( driver->peripheral->rx_dma_config.stream, driver->peripheral->rx_dma_config.complete_flags | driver->peripheral->rx_dma_config.error_flags );

    driver->peripheral->rx_dma_config.stream->NDTR  = size;
    driver->peripheral->rx_dma_config.stream->M0AR  = (uint32_t)data;
    driver->peripheral->rx_dma_config.stream->CR   |= DMA_SxCR_EN;
    USART_DMACmd( driver->peripheral->port, USART_DMAReq_Rx, ENABLE );

    if ( timeout > 0 )
    {
        result = ( platform_result_t ) host_rtos_get_semaphore( &driver->rx_complete, timeout, WICED_TRUE );
    }

    return result;
}

static void clear_dma_interrupts( DMA_Stream_TypeDef* stream, uint32_t flags )
{
    if ( stream <= DMA1_Stream3 )
    {
        DMA1->LIFCR |= flags;
    }
    else if ( stream <= DMA1_Stream7 )
    {
        DMA1->HIFCR |= flags;
    }
    else if ( stream <= DMA2_Stream3 )
    {
        DMA2->LIFCR |= flags;
    }
    else
    {
        DMA2->HIFCR |= flags;
    }
}

static uint32_t get_dma_irq_status( DMA_Stream_TypeDef* stream )
{
    if ( stream <= DMA1_Stream3 )
    {
        return DMA1->LISR;
    }
    else if ( stream <= DMA1_Stream7 )
    {
        return DMA1->HISR;
    }
    else if ( stream <= DMA2_Stream3 )
    {
        return DMA2->LISR;
    }
    else
    {
        return DMA2->HISR;
    }
}

/******************************************************
 *            IRQ Handlers Definition
 ******************************************************/

void platform_uart_irq( platform_uart_driver_t* driver )
{
    platform_uart_port_t* uart = (platform_uart_port_t*) driver->peripheral->port;

    // Clear all interrupts. It's safe to do so because only RXNE interrupt is enabled
    uart->SR = 0xffff;

    // Update tail
    driver->rx_buffer->tail = driver->rx_buffer->size - driver->peripheral->rx_dma_config.stream->NDTR;

    // Notify thread if sufficient data are available
    if ( ( driver->rx_size > 0 ) && ( ring_buffer_used_space( driver->rx_buffer ) >= driver->rx_size ) )
    {
        host_rtos_set_semaphore( &driver->rx_complete, WICED_TRUE );
        driver->rx_size = 0;
    }
}

void platform_uart_tx_dma_irq( platform_uart_driver_t* driver )
{
    if ( ( get_dma_irq_status( driver->peripheral->tx_dma_config.stream ) & driver->peripheral->tx_dma_config.complete_flags ) != 0 )
    {
        clear_dma_interrupts( driver->peripheral->tx_dma_config.stream, driver->peripheral->tx_dma_config.complete_flags );
        driver->last_transmit_result = PLATFORM_SUCCESS;
    }

    if ( ( get_dma_irq_status( driver->peripheral->tx_dma_config.stream ) & driver->peripheral->tx_dma_config.error_flags ) != 0 )
    {
        clear_dma_interrupts( driver->peripheral->tx_dma_config.stream, driver->peripheral->tx_dma_config.error_flags );
        driver->last_transmit_result = PLATFORM_ERROR;
    }

    if ( driver->tx_size > 0 )
    {
        /* Set semaphore regardless of result to prevent waiting thread from locking up */
        host_rtos_set_semaphore( &driver->tx_complete, WICED_TRUE );
    }
}

void platform_uart_rx_dma_irq( platform_uart_driver_t* driver )
{
    if ( ( get_dma_irq_status( driver->peripheral->rx_dma_config.stream ) & driver->peripheral->rx_dma_config.complete_flags ) != 0 )
    {
        clear_dma_interrupts( driver->peripheral->rx_dma_config.stream, driver->peripheral->rx_dma_config.complete_flags );
        driver->last_receive_result = PLATFORM_SUCCESS;
    }

    if ( ( get_dma_irq_status( driver->peripheral->rx_dma_config.stream ) & driver->peripheral->rx_dma_config.error_flags ) != 0 )
    {
        clear_dma_interrupts( driver->peripheral->rx_dma_config.stream, driver->peripheral->rx_dma_config.error_flags );
        driver->last_receive_result = PLATFORM_ERROR;
    }

    if ( driver->rx_size > 0 )
    {
        /* Set semaphore regardless of result to prevent waiting thread from locking up */
        host_rtos_set_semaphore( &driver->rx_complete, WICED_TRUE );
    }
}

platform_result_t platform_uart_powersave_wakeup_handler ( const platform_uart_t* peripheral )
{
    return PLATFORM_SUCCESS;
}

platform_result_t platform_uart_powersave_sleep_handler ( const platform_uart_t* peripheral )
{
    return PLATFORM_SUCCESS;
}
