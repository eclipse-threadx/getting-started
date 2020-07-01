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
#include <stdint.h>
#include <string.h>
#include "platform_cmsis.h"
#include "platform_constants.h"
#include "platform_peripheral.h"
#include "platform_isr_interface.h"
#include "wwd_assert.h"
#include "wiced_utilities.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define MAX_NUM_SPI_PRESCALERS          (8)
#define SPI_DMA_CTL_TIMEOUT_LOOPS       (1000)
#define DMA_MIN_TX                      (32)
#define DMA_MIN_RX                      (32)
#define DMA_MAX_SIZE                    (65536)

#define SPI_DATA_WIDTH_8_BITS           (8)
#define SPI_DATA_WIDTH_16_BITS          (16)
#define SPI_CRCPOLYNOMIAL_RESET_VALUE   (0x7)

#define SPI_INIT_DATA_BYTE              (0xFF)
#define SPI_INIT_DATA_WORD              (0xFFFF)

#define SPI_TRANS_STATUS_BYTE_ENABLE    (0)

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

typedef struct
{
    uint16_t factor;
    uint16_t prescaler_value;
} spi_baudrate_division_mapping_t;

typedef struct
{
    uint32_t speed;
    uint8_t mode;
    uint8_t bits;
    platform_gpio_t cs;
    wiced_bool_t spi_ready;
    platform_spi_port_t* spi;
} spi_current_config_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static platform_result_t platform_spi_slave_transfer_data_internal( platform_spi_slave_driver_t* driver, platform_spi_slave_transfer_direction_t direction, platform_spi_slave_data_buffer_t* buffer, uint32_t timeout_ms );

static platform_result_t calculate_prescaler( const platform_spi_port_t* spi, uint32_t speed, uint16_t* prescaler );
static uint16_t          spi_transfer       ( const platform_spi_t* spi, uint16_t data );
static platform_result_t spi_dma_transfer   ( const platform_spi_t* spi, const platform_spi_message_segment_t* message );
static platform_result_t spi_chip_select_toggle( const platform_spi_t* spi, const platform_spi_config_t* config, wiced_bool_t activate );

/******************************************************
 *               Variable Definitions
 ******************************************************/
uint32_t         dummy; // Dummy memory for RX DMA

static spi_current_config_t spi_current_config[NUMBER_OF_SPI_PORTS];

static const spi_baudrate_division_mapping_t spi_baudrate_prescalers[MAX_NUM_SPI_PRESCALERS] =
{
    { 2,   SPI_BaudRatePrescaler_2   },
    { 4,   SPI_BaudRatePrescaler_4   },
    { 8,   SPI_BaudRatePrescaler_8   },
    { 16,  SPI_BaudRatePrescaler_16  },
    { 32,  SPI_BaudRatePrescaler_32  },
    { 64,  SPI_BaudRatePrescaler_64  },
    { 128, SPI_BaudRatePrescaler_128 },
    { 256, SPI_BaudRatePrescaler_256 },
};

/* SPI peripheral clock functions */
static const platform_peripheral_clock_function_t spi_peripheral_clock_functions[NUMBER_OF_SPI_PORTS] =
{
    [0] = RCC_APB2PeriphClockCmd,
    [1] = RCC_APB1PeriphClockCmd,
    [2] = RCC_APB1PeriphClockCmd,
#if defined(STM32F401xx) || defined(STM32F446xx) || defined(STM32F411xE) || defined(STM32F427_437xx) ||\
    defined(STM32F429_439xx) || defined(STM32F412xG)
    [3] = RCC_APB2PeriphClockCmd,
#endif
#if defined(STM32F411xE) || defined(STM32F427_437xx) || defined(STM32F429_439xx) || defined(STM32F412xG)
    [4] = RCC_APB2PeriphClockCmd,
#endif
#if defined(STM32F427_437xx) || defined(STM32F429_439xx)
    [5] = RCC_APB2PeriphClockCmd,
#endif
};

/* SPI peripheral clocks */
static const uint32_t spi_peripheral_clocks[NUMBER_OF_SPI_PORTS] =
{
    [0] = RCC_APB2Periph_SPI1,
    [1] = RCC_APB1Periph_SPI2,
    [2] = RCC_APB1Periph_SPI3,
#if defined(STM32F401xx) || defined(STM32F446xx) || defined(STM32F411xE) || defined(STM32F427_437xx) ||\
    defined(STM32F429_439xx) || defined(STM32F412xG)
    [3] = RCC_APB2Periph_SPI4,
#endif
#if defined(STM32F411xE) || defined(STM32F427_437xx) || defined(STM32F429_439xx) || defined(STM32F412xG)
    [4] = RCC_APB2Periph_SPI5,
#endif
#if defined(STM32F427_437xx) || defined(STM32F429_439xx)
    [5] = RCC_APB2Periph_SPI6,
#endif
};

/******************************************************
 *               Function Definitions
 ******************************************************/

uint8_t platform_spi_get_port_number( platform_spi_port_t* spi )
{
    if ( spi == SPI1 )
    {
        return 0;
    }
    else if ( spi == SPI2 )
    {
        return 1;
    }
    else if ( spi == SPI3 )
    {
        return 2;
    }
#if defined(STM32F401xx) || defined(STM32F446xx) || defined(STM32F411xE) || defined(STM32F427_437xx) ||\
    defined(STM32F429_439xx) || defined(STM32F412xG)
    else if ( spi == SPI4 )
    {
        return 3;
    }
#endif
#if defined(STM32F411xE) || defined(STM32F427_437xx) || defined(STM32F429_439xx) || defined(STM32F412xG)
    else if ( spi == SPI5 )
    {
        return 4;
    }
#endif
#if defined(STM32F427_437xx) || defined(STM32F429_439xx)
    else if ( spi == SPI6 )
    {
        return 5;
    }
#endif
    else
    {
        return INVALID_UART_PORT_NUMBER;
    }
}

platform_result_t platform_spi_init( const platform_spi_t* spi, const platform_spi_config_t* config )
{
    SPI_InitTypeDef   spi_init;
    platform_result_t result;
    uint8_t           spi_number;

    wiced_assert( "bad argument", ( spi != NULL ) && ( config != NULL ) );

    /* Only configure if we have not done so already or parameters have changed */
    spi_number = platform_spi_get_port_number( spi->port );

    if (spi_current_config[spi_number].bits == config->bits &&
            spi_current_config[spi_number].cs.port == config->chip_select->port &&
            spi_current_config[spi_number].cs.pin_number == config->chip_select->pin_number &&
            spi_current_config[spi_number].mode == config->mode &&
            spi_current_config[spi_number].speed == config->speed &&
            spi_current_config[spi_number].spi == spi->port &&
            spi_current_config[spi_number].spi_ready == WICED_TRUE)
        return PLATFORM_SUCCESS;

    platform_mcu_powersave_disable();

#if defined SPLIT_SPI_CONFIG
    /* Init SPI GPIOs */
    platform_gpio_set_alternate_function( spi->pin_clock->port, spi->pin_clock->pin_number, GPIO_OType_PP, GPIO_PuPd_NOPULL, spi->gpio_af & 0xFF);
    platform_gpio_set_alternate_function( spi->pin_mosi->port,  spi->pin_mosi->pin_number,  GPIO_OType_PP, GPIO_PuPd_NOPULL, (spi->gpio_af >>  8) & 0xFF);
    platform_gpio_set_alternate_function( spi->pin_miso->port,  spi->pin_miso->pin_number,  GPIO_OType_PP, GPIO_PuPd_NOPULL, (spi->gpio_af >> 16) & 0xFF);
#else
    /* Init SPI GPIOs */
    platform_gpio_set_alternate_function( spi->pin_clock->port, spi->pin_clock->pin_number, GPIO_OType_PP, GPIO_PuPd_NOPULL, spi->gpio_af & 0xFF );
    platform_gpio_set_alternate_function( spi->pin_mosi->port,  spi->pin_mosi->pin_number,  GPIO_OType_PP, GPIO_PuPd_NOPULL, spi->gpio_af & 0xFF);
    platform_gpio_set_alternate_function( spi->pin_miso->port,  spi->pin_miso->pin_number,  GPIO_OType_PP, GPIO_PuPd_NOPULL, spi->gpio_af & 0xFF);
#endif

    /* Init the chip select GPIO */
    platform_gpio_init( config->chip_select, OUTPUT_PUSH_PULL );
    spi_chip_select_toggle( spi, config, WICED_FALSE );

    /* Calculate prescaler */
    result = calculate_prescaler( spi->port, config->speed, &spi_init.SPI_BaudRatePrescaler );
    if ( result != PLATFORM_SUCCESS )
    {
        platform_mcu_powersave_enable();
        return result;
    }

    /* Configure data-width */
    if ( config->bits == SPI_DATA_WIDTH_8_BITS )
    {
        spi_init.SPI_DataSize = SPI_DataSize_8b;
    }
    else if ( config->bits == SPI_DATA_WIDTH_16_BITS )
    {
        if ( config->mode & SPI_USE_DMA )
        {
            platform_mcu_powersave_enable();

            /* 16 bit mode is not supported for a DMA */
            return PLATFORM_UNSUPPORTED;
        }

        spi_init.SPI_DataSize = SPI_DataSize_16b;
    }
    else
    {
        platform_mcu_powersave_enable();

        /* Requested mode is not supported */
        return PLATFORM_UNSUPPORTED;
    }

    /* Configure MSB or LSB */
    if ( config->mode & SPI_MSB_FIRST )
    {
        spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
    }
    else
    {
        spi_init.SPI_FirstBit = SPI_FirstBit_LSB;
    }

    /* Configure mode CPHA and CPOL */
    if ( config->mode & SPI_CLOCK_IDLE_HIGH )
    {
        spi_init.SPI_CPOL = SPI_CPOL_High;
    }
    else
    {
        spi_init.SPI_CPOL = SPI_CPOL_Low;
    }

    if ( config->mode & SPI_CLOCK_RISING_EDGE )
    {
        spi_init.SPI_CPHA = ( config->mode & SPI_CLOCK_IDLE_HIGH ) ? SPI_CPHA_2Edge : SPI_CPHA_1Edge;
    }
    else
    {
        spi_init.SPI_CPHA = ( config->mode & SPI_CLOCK_IDLE_HIGH ) ? SPI_CPHA_1Edge : SPI_CPHA_2Edge;
    }

    /* Enable SPI peripheral clock */
    spi_peripheral_clock_functions[ spi_number ]( spi_peripheral_clocks[ spi_number ], ENABLE );

    spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_init.SPI_Mode      = SPI_Mode_Master;
    spi_init.SPI_NSS       = SPI_NSS_Soft;
    spi_init.SPI_CRCPolynomial = SPI_CRCPOLYNOMIAL_RESET_VALUE; /* reset value (0x7) */
    SPI_CalculateCRC( spi->port, DISABLE );

    /* Init and enable SPI */
    SPI_Init( spi->port, &spi_init );

    /* Save configuration to avoid having to do this every time */
    spi_current_config[spi_number].bits = config->bits;
    spi_current_config[spi_number].cs.port = config->chip_select->port;
    spi_current_config[spi_number].cs.pin_number = config->chip_select->pin_number;
    spi_current_config[spi_number].mode = config->mode;
    spi_current_config[spi_number].speed = config->speed;
    spi_current_config[spi_number].spi = spi->port;
    spi_current_config[spi_number].spi_ready = WICED_TRUE;

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_spi_deinit( const platform_spi_t* spi )
{
    UNUSED_PARAMETER( spi );
    /* TODO: unimplemented */
    return PLATFORM_SUCCESS;
}

platform_result_t platform_spi_transfer( const platform_spi_t* spi, const platform_spi_config_t* config, const platform_spi_message_segment_t* segments, uint16_t number_of_segments )
{
    platform_result_t result = PLATFORM_SUCCESS;
    uint32_t          count  = 0;
    uint16_t          i;

    wiced_assert( "bad argument", ( spi != NULL ) && ( config != NULL ) && ( segments != NULL ) && ( number_of_segments != 0 ) );

    if (( spi == NULL ) || ( config == NULL ) || ( segments == NULL ) || ( number_of_segments == 0 ) )
        return PLATFORM_BADARG;

    platform_mcu_powersave_disable();

    /* Enable port first */
    SPI_Cmd ( spi->port, ENABLE );
    /* Activate chip select */
    spi_chip_select_toggle( spi, config, WICED_TRUE );

    for ( i = 0; i < number_of_segments; i++ )
    {
        /* Check if we are using DMA */
        if ( config->mode & SPI_USE_DMA )
        {
            /* DMA is only efficient if we are using this for large transfers */
            if (segments[i].length == 0)
                continue; // Dummy segment
            /* Max DMA size */
            if (segments[i].length < DMA_MAX_SIZE)
            {
                if (((segments[i].tx_buffer != NULL) && (segments[i].length > DMA_MIN_TX)) ||
                        ((segments[i].rx_buffer != NULL) && (segments[i].length > DMA_MIN_RX)))
                {
                    result = spi_dma_transfer( spi, &segments[ i ] );

                    if ( result == PLATFORM_SUCCESS )
                        continue;
            }
        }
        }
        /* Fall back to non-DMA operations if either DMA is not enabled or DMA doesn't work correctly or if this is more efficient*/
        {
            count = segments[i].length;

            /* in interrupt-less mode */
            if ( config->bits == SPI_DATA_WIDTH_8_BITS )
            {
                const uint8_t* send_ptr = ( const uint8_t* )segments[i].tx_buffer;
                uint8_t*       rcv_ptr  = ( uint8_t* )segments[i].rx_buffer;

                while ( count-- )
                {
                    uint16_t data = SPI_INIT_DATA_BYTE;

                    if ( send_ptr != NULL )
                    {
                        data = *send_ptr++;
                    }

                    data = spi_transfer( spi, data );

                    if ( rcv_ptr != NULL )
                    {
                        *rcv_ptr++ = (uint8_t)data;
                    }
                }
            }
            else if ( config->bits == SPI_DATA_WIDTH_16_BITS )
            {
                const uint16_t* send_ptr = (const uint16_t *) segments[i].tx_buffer;
                uint16_t*       rcv_ptr  = (uint16_t *) segments[i].rx_buffer;

                /* Check that the message length is a multiple of 2 */
                if ( ( count % 2 ) != 0 )
                {
                    result = PLATFORM_ERROR;
                    goto cleanup_transfer;
                }

                /* Transmit/receive data stream, 16-bit at time */
                while ( count != 0 )
                {
                    uint16_t data = SPI_INIT_DATA_WORD;

                    if ( send_ptr != NULL )
                    {
                        data = *send_ptr++;
                    }

                    data = spi_transfer( spi, data );

                    if ( rcv_ptr != NULL )
                    {
                        *rcv_ptr++ = data;
                    }

                    count -= 2;
                }
            }
        }
    }

cleanup_transfer:

    /* Deassert chip select */
    spi_chip_select_toggle( spi, config, WICED_FALSE );

    SPI_Cmd ( spi->port, DISABLE );

    platform_mcu_powersave_enable( );

    return result;
}

static uint16_t spi_transfer( const platform_spi_t* spi, uint16_t data )
{
    /* Wait until the transmit buffer is empty */
    while ( SPI_I2S_GetFlagStatus( spi->port, SPI_I2S_FLAG_TXE ) == RESET )
    {
    }

    /* Send the byte */
    SPI_I2S_SendData( spi->port, data );

    /* Wait until a data is received */
    while ( SPI_I2S_GetFlagStatus( spi->port, SPI_I2S_FLAG_RXNE ) == RESET )
    {
    }

    while ( SPI_I2S_GetFlagStatus( spi->port, SPI_I2S_FLAG_BSY ) != RESET )
    {
    }


    /* Get the received data */
    return SPI_I2S_ReceiveData( spi->port );
}

static platform_result_t spi_chip_select_toggle( const platform_spi_t* spi, const platform_spi_config_t* config, wiced_bool_t activate )
{
    UNUSED_PARAMETER( spi );

    if ( config->chip_select != NULL )
    {
        if ( activate == WICED_TRUE )
        {
            if ( config->mode & SPI_CS_ACTIVE_HIGH )
            {
                platform_gpio_output_high( config->chip_select );
            }
            else
            {
                platform_gpio_output_low( config->chip_select );
            }
        }
        else
        {
            if ( config->mode & SPI_CS_ACTIVE_HIGH )
            {
                platform_gpio_output_low( config->chip_select );
            }
            else
            {
                platform_gpio_output_high( config->chip_select );
            }
        }
    }

    return PLATFORM_SUCCESS;
}


static platform_result_t calculate_prescaler( const platform_spi_port_t* SPIx, uint32_t speed, uint16_t* prescaler )
{
    RCC_ClocksTypeDef RCC_Clocks;
    uint32_t APB_Frequency;
    uint8_t i;

    wiced_assert("Bad args", prescaler != NULL);

    if (prescaler == NULL)
        return PLATFORM_BADARG;

    if (speed == 0)
    {
        *prescaler = spi_baudrate_prescalers[MAX_NUM_SPI_PRESCALERS-1].prescaler_value;
        return PLATFORM_SUCCESS;
    }

    RCC_GetClocksFreq( &RCC_Clocks );

    if (SPIx == SPI1 || SPIx == SPI4 || SPIx == SPI5 || SPIx == SPI6)
        APB_Frequency = RCC_Clocks.PCLK2_Frequency;
    else
        APB_Frequency = RCC_Clocks.PCLK1_Frequency;

    for (i = 0; i < MAX_NUM_SPI_PRESCALERS; i++) {
        if ((APB_Frequency / spi_baudrate_prescalers[i].factor ) <= speed) {
            *prescaler = spi_baudrate_prescalers[i].prescaler_value;
            return PLATFORM_SUCCESS;
        }
    }

    return PLATFORM_BADARG;
}

static platform_result_t spi_dma_transfer( const platform_spi_t* spi, const platform_spi_message_segment_t* message )
{
    DMA_InitTypeDef dma_init;
    uint32_t loop_count;
    platform_result_t result = PLATFORM_SUCCESS;

    /* Error check buffers */
    if ((message->rx_buffer == NULL) && (message->tx_buffer == NULL))
        return PLATFORM_ERROR;

    /* Enable DMA peripheral clock */
    if ( spi->tx_dma.controller == DMA1 )
    {
        RCC->AHB1ENR |= RCC_AHB1Periph_DMA1;
    }
    else
    {
        RCC->AHB1ENR |= RCC_AHB1Periph_DMA2;
    }

    dma_init.DMA_PeripheralBaseAddr = ( uint32_t )&spi->port->DR;
    dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_BufferSize         = message->length;
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init.DMA_Mode               = DMA_Mode_Normal;
    dma_init.DMA_Priority           = DMA_Priority_High;
    dma_init.DMA_FIFOMode           = DMA_FIFOMode_Disable;
    dma_init.DMA_FIFOThreshold      = DMA_FIFOThreshold_1QuarterFull;
    dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;

    /* Setup RX first */
    DMA_ClearFlag( spi->rx_dma.stream, spi->rx_dma.complete_flags | spi->rx_dma.error_flags );

    DMA_DeInit( spi->rx_dma.stream );

    loop_count = 0;
    while (DMA_GetCmdStatus( spi->rx_dma.stream ) == ENABLE)
    {
        loop_count++;
          if ( loop_count >= (uint32_t) SPI_DMA_CTL_TIMEOUT_LOOPS )
            return PLATFORM_TIMEOUT;
    }

    dma_init.DMA_Channel            = spi->rx_dma.channel;
    dma_init.DMA_DIR                = DMA_DIR_PeripheralToMemory;
    if (message->rx_buffer != NULL)
    {
        dma_init.DMA_Memory0BaseAddr    = (uint32_t)message->rx_buffer;
        dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    }
    else
    {
        dma_init.DMA_Memory0BaseAddr    = (uint32_t)&dummy; /* To support the TX only case */
        dma_init.DMA_MemoryInc          = DMA_MemoryInc_Disable;
    }

    /* Init and activate RX DMA channel */
    DMA_Init( spi->rx_dma.stream, &dma_init );

    DMA_Cmd( spi->rx_dma.stream, ENABLE );
    loop_count = 0;
    while (DMA_GetCmdStatus( spi->rx_dma.stream ) == DISABLE)
    {
        loop_count++;
        if ( loop_count >= (uint32_t) SPI_DMA_CTL_TIMEOUT_LOOPS )
        {
            return PLATFORM_TIMEOUT;
        }
    }

    /* Now the TX */
    DMA_ClearFlag( spi->tx_dma.stream, spi->tx_dma.complete_flags | spi->tx_dma.error_flags );

    DMA_DeInit( spi->tx_dma.stream );

    loop_count = 0;
    while (DMA_GetCmdStatus( spi->tx_dma.stream ) == ENABLE)
    {
        loop_count++;
        if ( loop_count >= (uint32_t) SPI_DMA_CTL_TIMEOUT_LOOPS )
        {
            return PLATFORM_TIMEOUT;
        }
    }

    /* Setup DMA stream for TX */
    dma_init.DMA_Channel            = spi->tx_dma.channel;
    dma_init.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
    if (message->tx_buffer != NULL)
    {
        dma_init.DMA_Memory0BaseAddr    = ( uint32_t )message->tx_buffer;
        dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    }
    else
    {
        dma_init.DMA_Memory0BaseAddr    = ( uint32_t )&dummy; /* To support the RX only case */
        dma_init.DMA_MemoryInc          = DMA_MemoryInc_Disable;
    }

    DMA_Init( spi->tx_dma.stream, &dma_init );

    DMA_Cmd( spi->tx_dma.stream, ENABLE );

    loop_count = 0;
    while (DMA_GetCmdStatus( spi->tx_dma.stream ) == DISABLE)
    {
        loop_count++;
        if ( loop_count >= (uint32_t) SPI_DMA_CTL_TIMEOUT_LOOPS )
        {
            return PLATFORM_TIMEOUT;
        }
    }

    /* Start both DMA together. TX is needed to drive the master clock for RX */
    SPI_I2S_DMACmd( spi->port, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE );

    /* Wait for DMAs done */
    while (DMA_GetCurrDataCounter(spi->tx_dma.stream) != 0)
    {
    }

    /* Strictly not necessary for RX since this should finish at the same time as TX */
    while (DMA_GetCurrDataCounter(spi->rx_dma.stream) != 0)
    {
    }

    /* Make sure the SPI is idle */
    while ( SPI_I2S_GetFlagStatus( spi->port, SPI_I2S_FLAG_TXE ) != SET )
    {
    }
    while ( SPI_I2S_GetFlagStatus( spi->port, SPI_I2S_FLAG_BSY ) != RESET )
    {
    }
    while ( SPI_I2S_GetFlagStatus( spi->port, SPI_I2S_FLAG_RXNE ) == SET )
    {
    }

    /* Clean up for next operation */
    SPI_I2S_DMACmd( spi->port, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE );
    DMA_Cmd(spi->tx_dma.stream, DISABLE);
    DMA_Cmd(spi->rx_dma.stream, DISABLE);

    return result;
}

platform_result_t platform_spi_slave_init( platform_spi_slave_driver_t* driver, const platform_spi_t* peripheral, const platform_spi_slave_config_t* config )
{
    SPI_InitTypeDef   spi_init;
    platform_result_t result;
    uint8_t           spi_number;

    wiced_assert( "bad argument", ( driver != NULL ) && ( peripheral != NULL ) && ( config != NULL ) );

    /* Only configure if we have not done so already or parameters have changed */
    spi_number = platform_spi_get_port_number( peripheral->port );

    platform_mcu_powersave_disable();

#if defined SPLIT_SPI_CONFIG
    /* Init SPI GPIOs */
    platform_gpio_set_alternate_function( peripheral->pin_clock->port, peripheral->pin_clock->pin_number, GPIO_OType_PP, GPIO_PuPd_NOPULL, peripheral->gpio_af & 0xFF);
    platform_gpio_set_alternate_function( peripheral->pin_mosi->port,  peripheral->pin_mosi->pin_number,  GPIO_OType_PP, GPIO_PuPd_NOPULL, (peripheral->gpio_af >>  8) & 0xFF);
    platform_gpio_set_alternate_function( peripheral->pin_miso->port,  peripheral->pin_miso->pin_number,  GPIO_OType_PP, GPIO_PuPd_NOPULL, (peripheral->gpio_af >> 16) & 0xFF);
#else
    /* Init SPI GPIOs */
    platform_gpio_set_alternate_function( peripheral->pin_clock->port, peripheral->pin_clock->pin_number, GPIO_OType_PP, GPIO_PuPd_NOPULL, peripheral->gpio_af & 0xFF );
    platform_gpio_set_alternate_function( peripheral->pin_mosi->port,  peripheral->pin_mosi->pin_number,  GPIO_OType_PP, GPIO_PuPd_NOPULL, peripheral->gpio_af & 0xFF);
    platform_gpio_set_alternate_function( peripheral->pin_miso->port,  peripheral->pin_miso->pin_number,  GPIO_OType_PP, GPIO_PuPd_NOPULL, peripheral->gpio_af & 0xFF);
#endif

    /* Calculate prescaler */
    result = calculate_prescaler( peripheral->port, config->speed, &spi_init.SPI_BaudRatePrescaler );
    if ( result != PLATFORM_SUCCESS )
    {
        platform_mcu_powersave_enable();
        return result;
    }

    /* Configure data-width */
    if ( config->bits == SPI_DATA_WIDTH_8_BITS )
    {
        spi_init.SPI_DataSize = SPI_DataSize_8b;
    }
    else if ( config->bits == SPI_DATA_WIDTH_16_BITS )
    {
        if ( config->mode & SPI_USE_DMA )
        {
            platform_mcu_powersave_enable();

            /* 16 bit mode is not supported for a DMA */
            return PLATFORM_UNSUPPORTED;
        }

        spi_init.SPI_DataSize = SPI_DataSize_16b;
    }
    else
    {
        platform_mcu_powersave_enable();

        /* Requested mode is not supported */
        return PLATFORM_UNSUPPORTED;
    }

    /* Configure MSB or LSB */
    if ( config->mode & SPI_MSB_FIRST )
    {
        spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
    }
    else
    {
        spi_init.SPI_FirstBit = SPI_FirstBit_LSB;
    }

    /* Configure mode CPHA and CPOL */
    if ( config->mode & SPI_CLOCK_IDLE_HIGH )
    {
        spi_init.SPI_CPOL = SPI_CPOL_High;
    }
    else
    {
        spi_init.SPI_CPOL = SPI_CPOL_Low;
    }

    if ( config->mode & SPI_CLOCK_RISING_EDGE )
    {
        spi_init.SPI_CPHA = ( config->mode & SPI_CLOCK_IDLE_HIGH ) ? SPI_CPHA_2Edge : SPI_CPHA_1Edge;
    }
    else
    {
        spi_init.SPI_CPHA = ( config->mode & SPI_CLOCK_IDLE_HIGH ) ? SPI_CPHA_1Edge : SPI_CPHA_2Edge;
    }

    /* Enable SPI peripheral clock */
    spi_peripheral_clock_functions[ spi_number ]( spi_peripheral_clocks[ spi_number ], ENABLE );

    spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_init.SPI_Mode      = SPI_Mode_Slave;
    spi_init.SPI_NSS       = SPI_NSS_Soft;
    spi_init.SPI_CRCPolynomial = SPI_CRCPOLYNOMIAL_RESET_VALUE; /* reset value (0x7) */
    SPI_CalculateCRC( peripheral->port, DISABLE );

    /* Init and enable SPI */
    SPI_Init( peripheral->port, &spi_init );

    driver->peripheral = peripheral;
    driver->config     = config;
    driver->state      = SPI_SLAVE_STATE_RECEIVING_COMMAND;

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_spi_slave_deinit( platform_spi_slave_driver_t* driver )
{
    return PLATFORM_SUCCESS;
}

platform_result_t platform_spi_slave_send_error_status( platform_spi_slave_driver_t* driver, platform_spi_slave_transfer_status_t error_status )
{
    platform_spi_slave_data_buffer_t buffer = { 0 };

    buffer.status      = error_status;
    buffer.data_length = 0;

    return platform_spi_slave_transfer_data_internal( driver, SPI_SLAVE_TRANSFER_WRITE, &buffer, WICED_NEVER_TIMEOUT );
}

platform_result_t platform_spi_slave_receive_command( platform_spi_slave_driver_t* driver, platform_spi_slave_command_t* command, uint32_t timeout_ms )
{
    return PLATFORM_SUCCESS;
}

static platform_result_t platform_spi_slave_transfer_data_internal( platform_spi_slave_driver_t* driver, platform_spi_slave_transfer_direction_t direction, platform_spi_slave_data_buffer_t* buffer, uint32_t timeout_ms )
{
    uint16_t tx_bytes_remaining;
    uint16_t rx_bytes_remaining;
    uint16_t tx_index;
    uint16_t rx_index;
    uint16_t tx_data_tmp = 0;
    uint16_t rx_data_tmp = 0;
    uint8_t tchar = SPI_INIT_DATA_BYTE;
    uint8_t rchar = 0;
    platform_spi_message_segment_t segment = {0};
    platform_result_t result = PLATFORM_SUCCESS;

    wiced_assert( "Invalid argument", driver != NULL );
    wiced_assert( "Invalid argument", buffer != NULL );
    wiced_assert( "Invalid argument", ( direction == SPI_SLAVE_TRANSFER_WRITE ) || ( direction == SPI_SLAVE_TRANSFER_READ ) );

    platform_mcu_powersave_disable();

    /* Enable port first */
    SPI_Cmd( driver->peripheral->port, ENABLE );

    driver->state = SPI_SLAVE_STATE_TRANSFERING_DATA;

    /* Check if we are using DMA */
    if ( driver->config->mode & SPI_USE_DMA )
    {
        STM32F4XX_PLATFORM_INFO( ("%s: Slave DMA mode transfer\n", __FUNCTION__) );

        if ( direction == SPI_SLAVE_TRANSFER_READ )
        {
            segment.tx_buffer   = NULL;
            segment.length      = buffer->data_length;
            segment.rx_buffer   = buffer->data;
        }
        else
        {
            segment.tx_buffer   = buffer->data;
            segment.length      = buffer->data_length;
            segment.rx_buffer   = NULL;
        }

        /* DMA is only efficient if we are using this for large transfers */
        if (segment.length == 0)
        {
            /* Dummy segment */
            goto cleanup_transfer;
        }

        /* Max DMA size */
        if (segment.length < DMA_MAX_SIZE)
        {
            if (((segment.tx_buffer != NULL) && (segment.length > DMA_MIN_TX)) ||
                    ((segment.rx_buffer != NULL) && (segment.length > DMA_MIN_RX)))
            {
                result = spi_dma_transfer( driver->peripheral, &segment );

                if ( result != PLATFORM_SUCCESS )
                {
                    STM32F4XX_PLATFORM_INFO( ("%s: spi_dma_transfer err! result=0x%x\n", __FUNCTION__, result) );
                }

                goto cleanup_transfer;
            }
        }

        STM32F4XX_PLATFORM_INFO( ("%s: Fall back to PIO operations!\n", __FUNCTION__) );
    }

    /* Fall back to non-DMA operations if either DMA is not enabled or DMA doesn't work correctly or if this is more efficient */
    {
        STM32F4XX_PLATFORM_INFO( ("%s: Slave PIO mode transfer\n", __FUNCTION__) );

        if ( direction == SPI_SLAVE_TRANSFER_READ )
        {
            tx_bytes_remaining = 0;
            rx_bytes_remaining = buffer->data_length;
        }
        else
        {
            tx_bytes_remaining = buffer->data_length;
            rx_bytes_remaining = 0;
        }

        tx_index = 0;
        rx_index = 0;

        STM32F4XX_PLATFORM_INFO( ("%s: tx_bytes_remaining %d, rx_bytes_remaining %d\n", __FUNCTION__, tx_bytes_remaining, rx_bytes_remaining) );

        while ( tx_bytes_remaining || rx_bytes_remaining )
        {
            tx_data_tmp = SPI_INIT_DATA_BYTE;
            rx_data_tmp = 0;

            if ( tx_bytes_remaining )
            {
#if (SPI_TRANS_STATUS_BYTE_ENABLE)
                if ( tx_index == 0 )
                {
                    tchar = buffer->status;
                }
                else
                {
                    if ( direction == SPI_SLAVE_TRANSFER_READ )
                    {
                        tchar = buffer->data[tx_index - 1];
                    }
                    else
                    {
                        tchar = SPI_INIT_DATA_BYTE;
                    }
                }
#else
                tchar = buffer->data[tx_index];
#endif

                /* Wait until the transmit buffer is empty */
                while ( SPI_I2S_GetFlagStatus( driver->peripheral->port, SPI_I2S_FLAG_TXE ) == RESET )
                {
                }

                tx_data_tmp = (uint16_t)tchar;
                SPI_I2S_SendData( driver->peripheral->port, tx_data_tmp );

                tx_bytes_remaining--;
                tx_index++;
            }

            if ( rx_bytes_remaining )
            {
                /* Wait until a data is received */
                while ( SPI_I2S_GetFlagStatus( driver->peripheral->port, SPI_I2S_FLAG_RXNE ) == RESET )
                {
                }

                rx_data_tmp = SPI_I2S_ReceiveData( driver->peripheral->port );

                rchar = (uint8_t)(rx_data_tmp & 0xFF);

#if (SPI_TRANS_STATUS_BYTE_ENABLE)
                if ( rx_index == 0 )
                {
                    buffer->status = rchar;
                }
                else
                {
                    buffer->data[rx_index - 1] = rchar;
                }
#else
                buffer->data[rx_index] = rchar;
#endif

                rx_bytes_remaining--;
                rx_index++;
            }
        }
    }


cleanup_transfer:

    driver->state = SPI_SLAVE_STATE_RECEIVING_COMMAND;

    /* Disable port */
    SPI_Cmd( driver->peripheral->port, DISABLE );

    platform_mcu_powersave_enable();

    return result;
}

platform_result_t platform_spi_slave_transfer_data( platform_spi_slave_driver_t* driver, platform_spi_slave_transfer_direction_t direction, platform_spi_slave_data_buffer_t* buffer, uint32_t timeout_ms )
{
    buffer->status = SPI_SLAVE_TRANSFER_SUCCESS;
    return platform_spi_slave_transfer_data_internal( driver, direction, buffer, timeout_ms );
}

platform_result_t platform_spi_slave_generate_interrupt( platform_spi_slave_driver_t* driver, uint32_t pulse_duration_ms )
{
    return PLATFORM_SUCCESS;
}

