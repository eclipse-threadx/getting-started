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
 *
 */
#include "stdint.h"
#include "string.h"
#include "platform_peripheral.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "wwd_rtos.h"
#include "wwd_assert.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define DMA_FLAG_TC(stream_id)           dma_flag_tc(stream_id)

/******************************************************
 *                    Constants
 ******************************************************/

#define I2C_FLAG_CHECK_TIMEOUT      ( 1000 )
#define I2C_FLAG_CHECK_LONG_TIMEOUT ( 1000 )
#define I2C_MESSAGE_DMA_MASK_POSN   ( 0 )
#define I2C_MESSAGE_NO_DMA          ( 0 << I2C_MESSAGE_DMA_MASK_POSN ) /* No DMA is set to 0 because DMA should be enabled by */
#define I2C_MESSAGE_USE_DMA         ( 1 << I2C_MESSAGE_DMA_MASK_POSN ) /* default, and turned off as an exception */
#define DMA_TIMEOUT_LOOPS           ( 10000000 )

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

static platform_result_t i2c_dma_config_and_execute( const platform_i2c_t* i2c, platform_i2c_message_t* message, wiced_bool_t tx_dma );
static platform_result_t i2c_dma_transfer( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message );
static platform_result_t i2c_rx_with_dma( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message );
static platform_result_t i2c_tx_with_dma( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message );
static platform_result_t i2c_transfer_message_no_dma( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message );
static platform_result_t i2c_address_device( const platform_i2c_t* i2c, const platform_i2c_config_t* config, int retries, uint8_t direction );
static platform_result_t i2c_tx_no_dma( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message );
static platform_result_t i2c_rx_no_dma( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message );
static platform_result_t i2c_wait_for_event( I2C_TypeDef* i2c, uint32_t event_id, uint32_t number_of_waits );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static DMA_InitTypeDef i2c_dma_init; /* Should investigate why this is global */

static const uint32_t dma_transfer_complete_flags[] =
{
    [0] = DMA_FLAG_TCIF0,
    [1] = DMA_FLAG_TCIF1,
    [2] = DMA_FLAG_TCIF2,
    [3] = DMA_FLAG_TCIF3,
    [4] = DMA_FLAG_TCIF4,
    [5] = DMA_FLAG_TCIF5,
    [6] = DMA_FLAG_TCIF6,
    [7] = DMA_FLAG_TCIF7,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_i2c_init( const platform_i2c_t* i2c, const platform_i2c_config_t* config )
{
    I2C_InitTypeDef I2C_InitStructure;

    wiced_assert( "bad argument", ( i2c != NULL ) && ( config != NULL ) );

    platform_mcu_powersave_disable( );

    // Init I2C GPIO clocks
    RCC_APB1PeriphClockCmd( i2c->peripheral_clock_reg, ENABLE );
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG,     ENABLE );

    // Enable Reset of the I2C clock around the GPIO configuration
    RCC_APB1PeriphResetCmd( i2c->peripheral_clock_reg, ENABLE );

    // GPIO Configuration
    platform_gpio_set_alternate_function( i2c->pin_scl->port, i2c->pin_scl->pin_number, GPIO_OType_OD, GPIO_PuPd_NOPULL, i2c->gpio_af );
    platform_gpio_set_alternate_function( i2c->pin_sda->port, i2c->pin_sda->pin_number, GPIO_OType_OD, GPIO_PuPd_NOPULL, i2c->gpio_af );

    if ( ( config->flags & I2C_DEVICE_USE_DMA ) != 0 )
    {
        // Enable the DMA clock
        RCC_AHB1PeriphClockCmd( i2c->tx_dma_peripheral_clock, ENABLE );

        // Configure the DMA streams for operation with the CP
        i2c_dma_init.DMA_Channel            = i2c->tx_dma_channel;
        i2c_dma_init.DMA_PeripheralBaseAddr = (uint32_t) &i2c->port->DR;
        i2c_dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
        i2c_dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
        i2c_dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        i2c_dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
        i2c_dma_init.DMA_Mode               = DMA_Mode_Normal;
        i2c_dma_init.DMA_Priority           = DMA_Priority_VeryHigh;
        //dma_init.DMA_FIFOMode             = DMA_FIFOMode_Enable;
        //dma_init.DMA_FIFOThreshold        = DMA_FIFOThreshold_Full;
        i2c_dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
        i2c_dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
        i2c_dma_init.DMA_Memory0BaseAddr    = (uint32_t) 0;               // This parameter will be configured during communication
        i2c_dma_init.DMA_DIR                = DMA_DIR_MemoryToPeripheral; // This parameter will be configured during communication
        i2c_dma_init.DMA_BufferSize         = 0xFFFF;                     // This parameter will be configured during communication

        DMA_DeInit( i2c->rx_dma_stream );
        DMA_DeInit( i2c->tx_dma_stream );

        // Clear any pending flags, disable, and clear the Tx DMA channel
        //DMA_ClearFlag( i2c_mapping[i2c->port].tx_dma_stream, CP_TX_DMA_FLAG_FEIF | CP_TX_DMA_FLAG_DMEIF | CP_TX_DMA_FLAG_TEIF | CP_TX_DMA_FLAG_HTIF | CP_TX_DMA_FLAG_TCIF );
        DMA_Cmd( i2c->tx_dma_stream, DISABLE );
        DMA_Cmd( i2c->rx_dma_stream, DISABLE );

        // Clear any pending flags, disable, and clear the Rx DMA channel
        //DMA_ClearFlag( i2c_mapping[i2c->port].rx_dma_stream, CP_RX_DMA_FLAG_FEIF | CP_RX_DMA_FLAG_DMEIF | CP_RX_DMA_FLAG_TEIF | CP_RX_DMA_FLAG_HTIF | CP_RX_DMA_FLAG_TCIF );
    }

    // Initialize the InitStruct for the CP
    I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1         = 0xA0;
    I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    if ( config->speed_mode == I2C_LOW_SPEED_MODE )
    {
        I2C_InitStructure.I2C_ClockSpeed = 10000;
    }
    else if ( config->speed_mode == I2C_STANDARD_SPEED_MODE )
    {
        I2C_InitStructure.I2C_ClockSpeed = 100000;
    }
    else if ( config->speed_mode == I2C_HIGH_SPEED_MODE )
    {
        I2C_InitStructure.I2C_ClockSpeed = 400000;
    }

    // Enable the I2C clock (turn off reset)
    RCC_APB1PeriphResetCmd( i2c->peripheral_clock_reg, DISABLE );

    // Enable and initialize the I2C bus
    I2C_Cmd( i2c->port, ENABLE );
    I2C_Init( i2c->port, &I2C_InitStructure );

    /* Enable DMA on the I2C bus if requested */
    if ( ( config->flags & I2C_DEVICE_USE_DMA ) != 0 )
    {
        I2C_DMACmd( i2c->port, ENABLE );
    }

    platform_mcu_powersave_enable( );

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_deinit( const platform_i2c_t* i2c, const platform_i2c_config_t* config )
{
    wiced_assert( "bad argument", ( i2c != NULL ) && ( config != NULL ) );

    platform_mcu_powersave_disable( );

    /* Disable I2C peripheral clocks */
    RCC_APB1PeriphClockCmd( i2c->peripheral_clock_reg, DISABLE );

    /* Disable DMA */
    if ( config->flags & I2C_DEVICE_USE_DMA )
    {
        DMA_DeInit( i2c->rx_dma_stream );
        DMA_DeInit( i2c->tx_dma_stream );
        RCC_AHB1PeriphClockCmd( i2c->tx_dma_peripheral_clock, DISABLE );
    }

    platform_mcu_powersave_enable( );

    return PLATFORM_SUCCESS;
}

wiced_bool_t platform_i2c_probe_device( const platform_i2c_t* i2c, const platform_i2c_config_t* config, int retries )
{
    platform_result_t result = PLATFORM_ERROR;

    wiced_assert( "bad argument", ( i2c != NULL ) && ( config != NULL ) );

    platform_mcu_powersave_disable();

    result = i2c_address_device( i2c, config, retries, I2C_Direction_Transmitter );

    I2C_GenerateSTOP( i2c->port, ENABLE );

    platform_mcu_powersave_enable();

    return ( result == PLATFORM_SUCCESS) ? WICED_TRUE : WICED_FALSE;
}

platform_result_t platform_i2c_init_tx_message( platform_i2c_message_t* message, const void* tx_buffer, uint16_t tx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    wiced_assert( "bad argument", ( message != NULL ) && ( tx_buffer != NULL ) && ( tx_buffer_length != 0 ) );

    memset( message, 0x00, sizeof( *message ) );
    message->tx_buffer = tx_buffer;
    message->retries   = retries;
    message->tx_length = tx_buffer_length;

    if ( disable_dma )
    {
        message->flags = I2C_MESSAGE_NO_DMA;
    }
    else
    {
        message->flags = I2C_MESSAGE_USE_DMA;
    }

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_init_rx_message( platform_i2c_message_t* message, void* rx_buffer, uint16_t rx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    wiced_assert( "bad argument", ( message != NULL ) && ( rx_buffer != NULL ) && ( rx_buffer_length != 0 ) );

    memset( message, 0x00, sizeof( *message ) );

    message->rx_buffer = rx_buffer;
    message->retries   = retries;
    message->rx_length = rx_buffer_length;

    if ( disable_dma )
    {
        message->flags = I2C_MESSAGE_NO_DMA;
    }
    else
    {
        message->flags = I2C_MESSAGE_USE_DMA;
    }

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_init_combined_message( platform_i2c_message_t* message, const void* tx_buffer, void* rx_buffer, uint16_t tx_buffer_length, uint16_t rx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    wiced_assert( "bad argument", ( message != NULL ) && ( tx_buffer != NULL ) && ( tx_buffer_length != 0 ) && ( rx_buffer != NULL ) && ( rx_buffer_length != 0 ) );

    memset( message, 0x00, sizeof( *message ) );

    message->rx_buffer = rx_buffer;
    message->tx_buffer = tx_buffer;
    message->retries   = retries;
    message->tx_length = tx_buffer_length;
    message->rx_length = rx_buffer_length;

    if ( disable_dma )
    {
        message->flags = I2C_MESSAGE_NO_DMA;
    }
    else
    {
        message->flags = I2C_MESSAGE_USE_DMA;
    }

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_transfer( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* messages, uint16_t number_of_messages )
{
    platform_result_t result = PLATFORM_ERROR;
    int               i      = 0;

    wiced_assert( "bad argument", ( i2c != NULL ) && ( config != NULL ) && ( messages != 0 ) && ( number_of_messages != 0 ) );

    platform_mcu_powersave_disable();

    for ( i = 0; i < number_of_messages; i++ )
    {
        if ( ( config->flags & I2C_DEVICE_USE_DMA ) && ( ( messages[ i ].flags & I2C_MESSAGE_USE_DMA ) == 1 ) )
        {
            result = i2c_dma_transfer( i2c, config, &messages[ i ] );
            if ( result != PLATFORM_SUCCESS )
            {
                goto exit;
            }
        }
        else
        {
            result = i2c_transfer_message_no_dma( i2c, config, &messages[ i ] );
            if ( result != PLATFORM_SUCCESS )
            {
                goto exit;
            }
        }
    }

exit:
    platform_mcu_powersave_enable();
    return result;
}

static platform_result_t i2c_dma_config_and_execute( const platform_i2c_t* i2c, platform_i2c_message_t* message, wiced_bool_t tx_dma )
{
    uint32_t counter;

    /* Initialize the DMA with the new parameters */
    if ( tx_dma == WICED_TRUE )
    {
        /* Enable DMA channel for I2C */
        I2C_DMACmd( i2c->port, ENABLE );

        /* TX DMA configuration */
        DMA_DeInit( i2c->tx_dma_stream );

        /* Configure the DMA TX Stream with the buffer address and the buffer size */
        i2c_dma_init.DMA_Memory0BaseAddr = (uint32_t) message->tx_buffer;
        i2c_dma_init.DMA_DIR             = DMA_DIR_MemoryToPeripheral;
        i2c_dma_init.DMA_BufferSize      = (uint32_t) message->tx_length;
        DMA_Init( i2c->tx_dma_stream, &i2c_dma_init );

        /* Enable DMA channel */
        DMA_Cmd( i2c->tx_dma_stream, ENABLE );

        /* wait until transfer is completed */
        /* TODO: change flag!!!!,wait on a semaphore */
        counter = DMA_TIMEOUT_LOOPS;
        while ( DMA_GetFlagStatus( i2c->tx_dma_stream, dma_transfer_complete_flags[ i2c->tx_dma_stream_id ] ) == RESET )
        {
            --counter;
            if ( counter == 0 )
            {
                return PLATFORM_ERROR;
            }
        }

        /* Disable DMA and channel */
        I2C_DMACmd( i2c->port, DISABLE );
        DMA_Cmd( i2c->tx_dma_stream, DISABLE );
    }
    else
    {
        /* Enable dma channel for I2C */
        I2C_DMACmd( i2c->port, ENABLE );

        /* RX DMA configuration */
        DMA_DeInit( i2c->rx_dma_stream );

        /* Configure the DMA Rx Stream with the buffer address and the buffer size */
        i2c_dma_init.DMA_Memory0BaseAddr = (uint32_t) message->rx_buffer;
        i2c_dma_init.DMA_DIR             = DMA_DIR_PeripheralToMemory;
        i2c_dma_init.DMA_BufferSize      = (uint32_t) message->rx_length;
        DMA_Init( i2c->rx_dma_stream, &i2c_dma_init );

        /* Enable DMA channel */
        DMA_Cmd( i2c->rx_dma_stream, ENABLE );

        /* wait until transfer is completed */
        counter = DMA_TIMEOUT_LOOPS;
        while ( DMA_GetFlagStatus( i2c->rx_dma_stream, dma_transfer_complete_flags[ i2c->rx_dma_stream_id ] ) == RESET )
        {
            --counter;
            if ( counter == 0 )
            {
                return PLATFORM_ERROR;
            }
        }

        /* disable DMA and channel */
        I2C_DMACmd( i2c->port, DISABLE );
        DMA_Cmd( i2c->rx_dma_stream, DISABLE );
    }

    return PLATFORM_SUCCESS;
}

static platform_result_t i2c_dma_transfer( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message )
{
    platform_result_t result;

    if ( message->tx_buffer != NULL )
    {
        result = i2c_tx_with_dma( i2c, config, message );
        if ( result != PLATFORM_SUCCESS )
        {
            goto exit;
        }
    }

    if ( message->rx_buffer != NULL )
    {
        result = i2c_rx_with_dma( i2c, config, message );
    }

exit:
    /* generate a stop condition */
    I2C_GenerateSTOP( i2c->port, ENABLE );

    return result;
}

static platform_result_t i2c_rx_with_dma( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message )
{
    uint32_t counter;

    i2c_address_device( i2c, config, message->retries, I2C_Direction_Receiver );

    /* Disable acknowledgement if receive a single byte */
    if ( message->rx_length == 1 )
    {
        I2C_AcknowledgeConfig( i2c->port, DISABLE );
    }
    else
    {
        /* enable acknowledgement before we start receiving multiple bytes */
        I2C_AcknowledgeConfig( i2c->port, ENABLE );
    }

    /* start dma which will read bytes */
    i2c_dma_config_and_execute( i2c, message, WICED_FALSE );

    /* wait til the last byte is received */
    counter = 1000;
    while ( I2C_GetFlagStatus( i2c->port, I2C_FLAG_BTF ) == RESET )
    {
        --counter;
        if ( counter == 0 )
        {
            return PLATFORM_ERROR;
        }
    }

    return PLATFORM_SUCCESS;
}

static platform_result_t i2c_tx_with_dma( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message )
{
    uint32_t counter;

    i2c_address_device(i2c, config, message->retries, I2C_Direction_Transmitter );

    /* Configure DMA tx channel for i2c */
    i2c_dma_config_and_execute( i2c, message, WICED_TRUE );

    /* Wait till the byte is actually sent from the i2c peripheral */
    counter = 1000;
    while ( I2C_GetFlagStatus( i2c->port, I2C_FLAG_BTF ) == RESET )
    {
        --counter;
        if ( counter == 0 )
        {
            return PLATFORM_ERROR;
        }
    }

    return PLATFORM_SUCCESS;
}

static platform_result_t i2c_transfer_message_no_dma( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message )
{
    platform_result_t result;

    if ( message->tx_buffer != NULL )
    {
        result = i2c_tx_no_dma( i2c, config, message );
        if ( result != PLATFORM_SUCCESS )
        {
            goto exit;
        }
    }

    if ( message->rx_buffer != NULL )
    {
        result = i2c_rx_no_dma( i2c, config, message );
    }

exit:
    /* generate a stop condition */
    I2C_GenerateSTOP( i2c->port, ENABLE );

    return result;

}

static platform_result_t i2c_address_device( const platform_i2c_t* i2c, const platform_i2c_config_t* config, int retries, uint8_t direction )
{
    platform_result_t result;

    /* Some chips( authentication and security related chips ) has to be addressed several times before they acknowledge their address */
    for ( ; retries != 0 ; --retries )
    {
        /* Generate a start condition and address a i2c in write mode */
        I2C_GenerateSTART( i2c->port, ENABLE );

        /* wait till start condition is generated and the bus becomes free */
        result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_MODE_SELECT, I2C_FLAG_CHECK_TIMEOUT );
        if ( result != PLATFORM_SUCCESS )
        {
            return PLATFORM_TIMEOUT;
        }

        if ( config->address_width == I2C_ADDRESS_WIDTH_7BIT )
        {
            /* send the address and R/W bit set to write of the requested i2c, wait for an acknowledge */
            I2C_Send7bitAddress( i2c->port, (uint8_t) ( config->address << 1 ), direction );

            /* wait till address gets sent and the direction bit is sent and */
            result = i2c_wait_for_event( i2c->port, (direction == I2C_Direction_Transmitter) ? I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED : I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, I2C_FLAG_CHECK_LONG_TIMEOUT );
            if ( result == PLATFORM_SUCCESS )
            {
                return PLATFORM_SUCCESS;
            }
        }

        /* TODO: Support other address widths */
    }

    return PLATFORM_TIMEOUT;
}

static platform_result_t i2c_tx_no_dma( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message )
{
    platform_result_t result;
    int               i;

    /* Send data */
    result = i2c_address_device( i2c, config, message->retries, I2C_Direction_Transmitter );
    if ( result != PLATFORM_SUCCESS )
    {
        return result;
    }

    for ( i = 0; i < message->tx_length; i++ )
    {
        I2C_SendData( i2c->port, ((uint8_t*)message->tx_buffer)[ i ] );

        /* wait till it actually gets transferred and acknowledged */
        result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_FLAG_CHECK_TIMEOUT );
        if ( result != PLATFORM_SUCCESS )
        {
            return result;
        }
    }

    return result;
}

static platform_result_t i2c_rx_no_dma( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message )
{
    platform_result_t result;
    int               i;

    result = i2c_address_device( i2c, config, message->retries, I2C_Direction_Receiver );
    if ( result != PLATFORM_SUCCESS )
    {
        return result;
    }

    /* Disable acknowledgement if this is a single byte transmission */
    if ( message->rx_length == 1 )
    {
        I2C_AcknowledgeConfig( i2c->port, DISABLE );
    }
    else
    {
        I2C_AcknowledgeConfig( i2c->port, ENABLE );
    }

    /* Start reading bytes */
    for ( i = 0; i < message->rx_length; i++ )
    {
        /* wait till something is in the i2c data register */
        result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_BYTE_RECEIVED, I2C_FLAG_CHECK_TIMEOUT );
        if ( result != PLATFORM_SUCCESS )
        {
            return result;
        }

        /* get data */
        ((uint8_t*)message->rx_buffer)[ i ] = I2C_ReceiveData( i2c->port );

        /* Check if last byte has been received */
        if ( i == ( message->rx_length - 1 ) )
        {
        }
        else /* Check if the second last byte has been received */
        if ( i == ( message->rx_length - 2 ) )
        {
            /* setup NACK for the last byte to be received */
            I2C_AcknowledgeConfig( i2c->port, DISABLE );
        }
        else
        {
            /* setup an acknowledgement beforehand for every byte that is to be received */
            I2C_AcknowledgeConfig( i2c->port, ENABLE );
        }
    }

    return result;
}

static platform_result_t i2c_wait_for_event( I2C_TypeDef* i2c, uint32_t event_id, uint32_t number_of_waits )
{
    while ( I2C_CheckEvent( i2c, event_id ) != SUCCESS )
    {
        number_of_waits--;
        if ( number_of_waits == 0 )
        {
            return PLATFORM_TIMEOUT;
        }
    }

    return PLATFORM_SUCCESS;
}
