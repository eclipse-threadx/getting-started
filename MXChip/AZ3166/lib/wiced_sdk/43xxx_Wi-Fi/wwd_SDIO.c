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
 * Defines WWD SDIO functions for STM32F4xx MCU
 */
#include <string.h> /* For memcpy */
#include "wwd_platform_common.h"
#include "wwd_bus_protocol.h"
#include "wwd_assert.h"
#include "platform/wwd_platform_interface.h"
#include "platform/wwd_sdio_interface.h"
#include "platform/wwd_bus_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "network/wwd_network_constants.h"
#include "misc.h"
#include "platform_cmsis.h"
#include "platform_peripheral.h"
#include "platform_config.h"
#include "wwd_rtos_isr.h"

/******************************************************
 *             Constants
 ******************************************************/

#define COMMAND_FINISHED_CMD52_TIMEOUT_LOOPS (100000)
#define COMMAND_FINISHED_CMD53_TIMEOUT_LOOPS (100000)
#define SDIO_TX_RX_COMPLETE_TIMEOUT_LOOPS    (100000)
#define SDIO_DMA_TIMEOUT_LOOPS               (1000000)
#define MAX_TIMEOUTS                         (30)
#if defined(STM32F412xG)
#define SDIO_ERROR_MASK                      ( SDIO_STA_DCRCFAIL | SDIO_STA_CTIMEOUT | SDIO_STA_DTIMEOUT | SDIO_STA_TXUNDERR | SDIO_STA_RXOVERR | SDIO_STA_STBITERR )
#else
#define SDIO_ERROR_MASK                      ( SDIO_STA_CCRCFAIL | SDIO_STA_DCRCFAIL | SDIO_STA_CTIMEOUT | SDIO_STA_DTIMEOUT | SDIO_STA_TXUNDERR | SDIO_STA_RXOVERR | SDIO_STA_STBITERR )
#endif
#define SDIO_IRQ_CHANNEL                     ((u8)0x31)
#define DMA2_3_IRQ_CHANNEL                   ((u8)DMA2_Stream3_IRQn)
#define BUS_LEVEL_MAX_RETRIES                (5)
#define SDIO_ENUMERATION_TIMEOUT_MS          (500)

/******************************************************
 *             Structures
 ******************************************************/

typedef struct
{
    /*@shared@*/ /*@null@*/ uint8_t* data;
    uint16_t length;
} sdio_dma_segment_t;

/******************************************************
 *             Variables
 ******************************************************/

static const uint32_t bus_direction_mapping[] =
{
    [BUS_READ]  = SDIO_TransferDir_ToSDIO,
    [BUS_WRITE] = SDIO_TransferDir_ToCard
};

ALIGNED_PRE(4) static uint8_t       temp_dma_buffer[MAX(2*1024,WICED_LINK_MTU+ 64)] ALIGNED(4);
static uint8_t*                     user_data;
static uint32_t                     user_data_size;
static uint8_t*                     dma_data_source;
static uint32_t                     dma_transfer_size;
static host_semaphore_type_t        sdio_transfer_finished_semaphore;
static wiced_bool_t                 sdio_transfer_failed;
static wwd_bus_transfer_direction_t current_transfer_direction;
static uint32_t                     current_command;

/******************************************************
 *             Static Function Declarations
 ******************************************************/

static uint32_t          sdio_get_blocksize_dctrl   ( sdio_block_size_t block_size );
static sdio_block_size_t find_optimal_block_size    ( uint32_t data_size );
static void              sdio_prepare_data_transfer ( wwd_bus_transfer_direction_t direction, sdio_block_size_t block_size, /*@unique@*/ uint8_t* data, uint16_t data_size ) /*@modifies dma_data_source, user_data, user_data_size, dma_transfer_size@*/;

/******************************************************
 *             Function definitions
 ******************************************************/

#ifndef  WICED_DISABLE_MCU_POWERSAVE
static void sdio_oob_irq_handler( void* arg )
{
    UNUSED_PARAMETER(arg);
    WWD_BUS_STATS_INCREMENT_VARIABLE( oob_intrs );
    platform_mcu_powersave_exit_notify( );
    wwd_thread_notify_irq( );
}
#endif /* ifndef  WICED_DISABLE_MCU_POWERSAVE */

static void sdio_enable_bus_irq( void )
{
#if defined(STM32F412xG)
    SDIO->MASK = SDIO_MASK_CMDRENDIE | SDIO_MASK_CMDSENTIE;
#else
    SDIO->MASK = SDIO_MASK_SDIOITIE | SDIO_MASK_CMDRENDIE | SDIO_MASK_CMDSENTIE;
#endif
}

static void sdio_disable_bus_irq( void )
{
    SDIO->MASK = 0;
}

#ifndef WICED_DISABLE_MCU_POWERSAVE
wwd_result_t host_enable_oob_interrupt( void )
{
    /* Set GPIO_B[1:0] to input. One of them will be re-purposed as OOB interrupt */
    platform_gpio_init( &wifi_sdio_pins[WWD_PIN_SDIO_OOB_IRQ], INPUT_HIGH_IMPEDANCE );
    platform_gpio_irq_enable( &wifi_sdio_pins[WWD_PIN_SDIO_OOB_IRQ], IRQ_TRIGGER_RISING_EDGE, sdio_oob_irq_handler, 0 );
    return WWD_SUCCESS;
}

uint8_t host_platform_get_oob_interrupt_pin( void )
{
    return WICED_WIFI_OOB_IRQ_GPIO_PIN;
}
#endif /* ifndef  WICED_DISABLE_MCU_POWERSAVE */

wwd_result_t host_platform_bus_init( void )
{
    SDIO_InitTypeDef sdio_init_structure;
    wwd_result_t     result;
    uint8_t          a;

    platform_mcu_powersave_disable();

    result = host_rtos_init_semaphore( &sdio_transfer_finished_semaphore );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    /* Turn on SDIO IRQ */
    SDIO->ICR = (uint32_t) 0xffffffff;

    /* Must be lower priority than the value of configMAX_SYSCALL_INTERRUPT_PRIORITY */
    /* otherwise FreeRTOS will not be able to mask the interrupt */
    /* keep in mind that ARMCM3 interrupt priority logic is inverted, the highest value */
    /* is the lowest priority */
    NVIC_EnableIRQ( ( IRQn_Type ) SDIO_IRQ_CHANNEL );
    NVIC_EnableIRQ( ( IRQn_Type ) DMA2_3_IRQ_CHANNEL );

#ifdef WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP_0
    /* Set GPIO_B[1:0] to 00 to put WLAN module into SDIO mode */
    platform_gpio_init( &wifi_control_pins[WWD_PIN_BOOTSTRAP_0], OUTPUT_PUSH_PULL );
    platform_gpio_output_low( &wifi_control_pins[WWD_PIN_BOOTSTRAP_0] );
#endif
#ifdef WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP_1
    platform_gpio_init( &wifi_control_pins[WWD_PIN_BOOTSTRAP_1], OUTPUT_PUSH_PULL );
    platform_gpio_output_low( &wifi_control_pins[WWD_PIN_BOOTSTRAP_1] );
#endif

    /* Setup GPIO pins for SDIO data & clock */
    for ( a = WWD_PIN_SDIO_CLK; a < WWD_PIN_SDIO_MAX; a++ )
    {
        platform_gpio_set_alternate_function( wifi_sdio_pins[ a ].port, wifi_sdio_pins[ a ].pin_number, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_AF_SDIO );
    }

    /*!< Enable the SDIO AHB Clock and the DMA2 Clock */
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA2, ENABLE );
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SDIO, ENABLE );

    SDIO_DeInit( );
    sdio_init_structure.SDIO_ClockDiv            = (uint8_t) 120; /* 0x78, clock is taken from the high speed APB bus ; */ /* About 400KHz */
    sdio_init_structure.SDIO_ClockEdge           = SDIO_ClockEdge_Rising;
    sdio_init_structure.SDIO_ClockBypass         = SDIO_ClockBypass_Disable;
    sdio_init_structure.SDIO_ClockPowerSave      = SDIO_ClockPowerSave_Enable;
    sdio_init_structure.SDIO_BusWide             = SDIO_BusWide_1b;
    sdio_init_structure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
    SDIO_Init( &sdio_init_structure );
    SDIO_SetPowerState( SDIO_PowerState_ON );
    SDIO_SetSDIOReadWaitMode( SDIO_ReadWaitMode_CLK );
    SDIO_ClockCmd( ENABLE );

    platform_mcu_powersave_enable();

    return WWD_SUCCESS;
}

wwd_result_t host_platform_sdio_enumerate( void )
{
    wwd_result_t result;
    uint32_t       loop_count;
    uint32_t       data = 0;

    loop_count = 0;
    do
    {
        /* Send CMD0 to set it to idle state */
        host_platform_sdio_transfer( BUS_WRITE, SDIO_CMD_0, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, NO_RESPONSE, NULL );

        /* CMD5. */
        host_platform_sdio_transfer( BUS_READ, SDIO_CMD_5, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, NO_RESPONSE, NULL );

        /* Send CMD3 to get RCA. */
        result = host_platform_sdio_transfer( BUS_READ, SDIO_CMD_3, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, RESPONSE_NEEDED, &data );
        loop_count++;
        if ( loop_count >= (uint32_t) SDIO_ENUMERATION_TIMEOUT_MS )
        {
            return WWD_TIMEOUT;
        }
    } while ( ( result != WWD_SUCCESS ) && ( host_rtos_delay_milliseconds( (uint32_t) 1 ), ( 1 == 1 ) ) );
    /* If you're stuck here, check the platform matches your hardware */

    /* Send CMD7 with the returned RCA to select the card */
    host_platform_sdio_transfer( BUS_WRITE, SDIO_CMD_7, SDIO_BYTE_MODE, SDIO_1B_BLOCK, data, 0, 0, RESPONSE_NEEDED, NULL );

    return WWD_SUCCESS;
}

wwd_result_t host_platform_bus_deinit( void )
{
    wwd_result_t result;
    uint32_t     a;

    result = host_rtos_deinit_semaphore( &sdio_transfer_finished_semaphore );

    platform_mcu_powersave_disable();

    /* Disable SPI and SPI DMA */
    sdio_disable_bus_irq( );
    SDIO_ClockCmd( DISABLE );
    SDIO_SetPowerState( SDIO_PowerState_OFF );
    SDIO_DeInit( );
//    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA2, DISABLE );
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SDIO, DISABLE );

    for ( a = 0; a < WWD_PIN_SDIO_MAX; a++ )
    {
        platform_gpio_deinit( &wifi_sdio_pins[ a ] );
    }

#ifdef WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP_0
    platform_gpio_deinit( &wifi_control_pins[WWD_PIN_BOOTSTRAP_0] );
#endif
#ifdef WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP_1
    platform_gpio_deinit( &wifi_control_pins[WWD_PIN_BOOTSTRAP_1] );
#endif

    /* Turn off SDIO IRQ */
    NVIC_DisableIRQ( ( IRQn_Type ) SDIO_IRQ_CHANNEL );
    NVIC_DisableIRQ( ( IRQn_Type ) DMA2_3_IRQ_CHANNEL );

    platform_mcu_powersave_enable();

    return result;
}

wwd_result_t host_platform_sdio_transfer( wwd_bus_transfer_direction_t direction, sdio_command_t command, sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t argument, /*@null@*/ uint32_t* data, uint16_t data_size, sdio_response_needed_t response_expected, /*@out@*/ /*@null@*/ uint32_t* response )
{
    uint32_t loop_count = 0;
    wwd_result_t result;
    uint16_t attempts = 0;

    wiced_assert("Bad args", !((command == SDIO_CMD_53) && (data == NULL)));

    if ( response != NULL )
    {
        *response = 0;
    }

    platform_mcu_powersave_disable();

    /* Ensure the bus isn't stuck half way through transfer */
    DMA2_Stream3->CR   = 0;

restart:
    SDIO->ICR = (uint32_t) 0xFFFFFFFF;
    sdio_transfer_failed = WICED_FALSE;
    ++attempts;

    /* Check if we've tried too many times */
    if (attempts >= (uint16_t) BUS_LEVEL_MAX_RETRIES)
    {
        result = WWD_SDIO_RETRIES_EXCEEDED;
        goto exit;
    }

    /* Prepare the data transfer register */
    current_command = command;
    if ( command == SDIO_CMD_53 )
    {
        sdio_enable_bus_irq();

        /* Dodgy STM32 hack to set the CMD53 byte mode size to be the same as the block size */
        if ( mode == SDIO_BYTE_MODE )
        {
            block_size = find_optimal_block_size( data_size );
            if ( block_size < SDIO_512B_BLOCK )
            {
                argument = ( argument & (uint32_t) ( ~0x1FF ) ) | block_size;
            }
            else
            {
                argument = ( argument & (uint32_t) ( ~0x1FF ) );
            }
        }

        /* Prepare the SDIO for a data transfer */
        current_transfer_direction = direction;
        sdio_prepare_data_transfer( direction, block_size, (uint8_t*) data, data_size );

        /* Send the command */
        SDIO->ARG = argument;
        SDIO->CMD = (uint32_t) ( command | SDIO_Response_Short | SDIO_Wait_No | SDIO_CPSM_Enable );

        /* Wait for the whole transfer to complete */
        result = host_rtos_get_semaphore( &sdio_transfer_finished_semaphore, (uint32_t) 50, WICED_TRUE );
        if ( result != WWD_SUCCESS )
        {
            goto exit;
        }

        if ( sdio_transfer_failed == WICED_TRUE )
        {
            goto restart;
        }

        /* Check if there were any SDIO errors */
        if ( ( SDIO->STA & ( SDIO_STA_DTIMEOUT | SDIO_STA_CTIMEOUT ) ) != 0 )
        {
            goto restart;
        }
        else if ( ( ( SDIO->STA & ( SDIO_STA_CCRCFAIL | SDIO_STA_DCRCFAIL | SDIO_STA_TXUNDERR | SDIO_STA_RXOVERR ) ) != 0 ) )
        {
            wiced_assert( "SDIO communication failure", 0 );
            goto restart;
        }

        /* Wait till complete */
        loop_count = (uint32_t) SDIO_TX_RX_COMPLETE_TIMEOUT_LOOPS;
        do
        {
            loop_count--;
            if ( loop_count == 0 || ( ( SDIO->STA & SDIO_ERROR_MASK ) != 0 ) )
            {
                goto restart;
            }
        } while ( ( SDIO->STA & ( SDIO_STA_TXACT | SDIO_STA_RXACT ) ) != 0 );

        if ( direction == BUS_READ )
        {
            memcpy( user_data, dma_data_source, (size_t) user_data_size );
        }
    }
    else
    {
        uint32_t temp_sta;

        /* Send the command */
        SDIO->ARG = argument;
        SDIO->CMD = (uint32_t) ( command | SDIO_Response_Short | SDIO_Wait_No | SDIO_CPSM_Enable );

        loop_count = (uint32_t) COMMAND_FINISHED_CMD52_TIMEOUT_LOOPS;
        do
        {
            temp_sta = SDIO->STA;
            loop_count--;
            if ( loop_count == 0 || ( ( response_expected == RESPONSE_NEEDED ) && ( ( temp_sta & SDIO_ERROR_MASK ) != 0 ) ) )
            {
                goto restart;
            }
        } while ( ( temp_sta & SDIO_FLAG_CMDACT ) != 0 );
#if defined(STM32F412xG)
        /* Errata */
        if (command == SDIO_CMD_5)
            SDIO->ICR = SDIO_ICR_CCRCFAILC;
#endif
    }

    if ( response != NULL )
    {
        *response = SDIO->RESP1;
    }
    result = WWD_SUCCESS;

exit:
    platform_mcu_powersave_enable();
//#if !defined(STM32F412xG)
    SDIO->MASK = SDIO_MASK_SDIOITIE;
//#endif
    return result;
}

static void sdio_prepare_data_transfer( wwd_bus_transfer_direction_t direction, sdio_block_size_t block_size, /*@unique@*/ uint8_t* data, uint16_t data_size ) /*@modifies dma_data_source, user_data, user_data_size, dma_transfer_size@*/
{
    /* Setup a single transfer using the temp buffer */
    user_data         = data;
    user_data_size    = data_size;
    dma_transfer_size = (uint32_t) ( ( ( data_size + (uint16_t) block_size - 1 ) / (uint16_t) block_size ) * (uint16_t) block_size );

    if ( direction == BUS_WRITE )
    {
        dma_data_source = data;
    }
    else
    {
        dma_data_source = temp_dma_buffer;
    }

    SDIO->DTIMER = (uint32_t) 0xFFFFFFFF;
    SDIO->DLEN   = dma_transfer_size;
    SDIO->DCTRL  = (uint32_t)sdio_get_blocksize_dctrl(block_size) | bus_direction_mapping[(int)direction] | SDIO_TransferMode_Block | SDIO_DPSM_Enable | (1 << 3) | (1 << 11);

    /* DMA2 Stream3 */
    DMA2_Stream3->CR   = 0;
    DMA2->LIFCR        = (uint32_t) ( 0x3F << 22 );
    DMA2_Stream3->FCR  = (uint32_t) ( 0x00000021 | DMA_FIFOMode_Enable | DMA_FIFOThreshold_Full );
    DMA2_Stream3->PAR  = (uint32_t) &SDIO->FIFO;
    DMA2_Stream3->M0AR = (uint32_t) dma_data_source;
    DMA2_Stream3->NDTR = dma_transfer_size/4;
}

wwd_result_t host_platform_enable_high_speed_sdio( void )
{
    SDIO_InitTypeDef sdio_init_structure;

#ifdef SLOW_SDIO_CLOCK
    sdio_init_structure.SDIO_ClockDiv       = (uint8_t) 1; /* 1 = 16MHz if SDIO clock = 48MHz */
#else
    sdio_init_structure.SDIO_ClockDiv       = (uint8_t) 0; /* 0 = 24MHz if SDIO clock = 48MHz */
#endif
    sdio_init_structure.SDIO_ClockEdge      = SDIO_ClockEdge_Rising;
    sdio_init_structure.SDIO_ClockBypass    = SDIO_ClockBypass_Disable;
    sdio_init_structure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
#ifndef SDIO_1_BIT
    sdio_init_structure.SDIO_BusWide = SDIO_BusWide_4b;
#else
    sdio_init_structure.SDIO_BusWide = SDIO_BusWide_1b;
#endif
    sdio_init_structure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;

    SDIO_DeInit( );
    SDIO_Init( &sdio_init_structure );
    SDIO_SetPowerState( SDIO_PowerState_ON );
    SDIO_ClockCmd( ENABLE );
    sdio_enable_bus_irq( );
    return WWD_SUCCESS;
}

static sdio_block_size_t find_optimal_block_size( uint32_t data_size )
{
    if ( data_size > (uint32_t) 256 )
        return SDIO_512B_BLOCK;
    if ( data_size > (uint32_t) 128 )
        return SDIO_256B_BLOCK;
    if ( data_size > (uint32_t) 64 )
        return SDIO_128B_BLOCK;
    if ( data_size > (uint32_t) 32 )
        return SDIO_64B_BLOCK;
    if ( data_size > (uint32_t) 16 )
        return SDIO_32B_BLOCK;
    if ( data_size > (uint32_t) 8 )
        return SDIO_16B_BLOCK;
    if ( data_size > (uint32_t) 4 )
        return SDIO_8B_BLOCK;
    if ( data_size > (uint32_t) 2 )
        return SDIO_4B_BLOCK;

    return SDIO_4B_BLOCK;
}

static uint32_t sdio_get_blocksize_dctrl(sdio_block_size_t block_size)
{
    switch (block_size)
    {
        case SDIO_1B_BLOCK:    return SDIO_DataBlockSize_1b;
        case SDIO_2B_BLOCK:    return SDIO_DataBlockSize_2b;
        case SDIO_4B_BLOCK:    return SDIO_DataBlockSize_4b;
        case SDIO_8B_BLOCK:    return SDIO_DataBlockSize_8b;
        case SDIO_16B_BLOCK:   return SDIO_DataBlockSize_16b;
        case SDIO_32B_BLOCK:   return SDIO_DataBlockSize_32b;
        case SDIO_64B_BLOCK:   return SDIO_DataBlockSize_64b;
        case SDIO_128B_BLOCK:  return SDIO_DataBlockSize_128b;
        case SDIO_256B_BLOCK:  return SDIO_DataBlockSize_256b;
        case SDIO_512B_BLOCK:  return SDIO_DataBlockSize_512b;
        case SDIO_1024B_BLOCK: return SDIO_DataBlockSize_1024b;
        case SDIO_2048B_BLOCK: return SDIO_DataBlockSize_2048b;
        default: return 0;
    }
}

wwd_result_t host_platform_bus_enable_interrupt( void )
{
    return  WWD_SUCCESS;
}

wwd_result_t host_platform_bus_disable_interrupt( void )
{
    return  WWD_SUCCESS;
}

void host_platform_bus_buffer_freed( wwd_buffer_dir_t direction )
{
    UNUSED_PARAMETER( direction );
}

/******************************************************
 *             IRQ Handler Definitions
 ******************************************************/

WWD_RTOS_DEFINE_ISR( SDIO_IRQHandler )
{
    uint32_t intstatus = SDIO->STA;

    WWD_BUS_STATS_INCREMENT_VARIABLE( sdio_intrs );
#if defined(STM32F412xG)
    if (current_command == SDIO_CMD_5)
        SDIO->ICR = SDIO_ICR_CCRCFAILC;
#endif
    if ( ( intstatus & ( SDIO_STA_CCRCFAIL | SDIO_STA_DCRCFAIL | SDIO_STA_TXUNDERR | SDIO_STA_RXOVERR  | SDIO_STA_STBITERR )) != 0 )
    {
        WWD_BUS_STATS_INCREMENT_VARIABLE( error_intrs );
        //wiced_assert("sdio error flagged",0);
        sdio_transfer_failed = WICED_TRUE;
        SDIO->ICR = (uint32_t) 0xffffffff;
        host_rtos_set_semaphore( &sdio_transfer_finished_semaphore, WICED_TRUE );
    }
    else
    {
        if ((intstatus & (SDIO_STA_CMDREND | SDIO_STA_CMDSENT)) != 0)
        {
            if ( ( SDIO->RESP1 & 0x800 ) != 0 )
            {
                sdio_transfer_failed = WICED_TRUE;
                host_rtos_set_semaphore( &sdio_transfer_finished_semaphore, WICED_TRUE );
            }
            else if (current_command == SDIO_CMD_53)
            {
                if (current_transfer_direction == BUS_WRITE)
                {
                    DMA2_Stream3->CR = DMA_DIR_MemoryToPeripheral |
                                       DMA_Channel_4 | DMA_PeripheralInc_Disable | DMA_MemoryInc_Enable |
                                       DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
                                       DMA_Mode_Normal | DMA_Priority_VeryHigh |
                                       DMA_MemoryBurst_INC4 | DMA_PeripheralBurst_INC4 | DMA_SxCR_PFCTRL | DMA_SxCR_EN | DMA_SxCR_TCIE;
                }
                else
                {
                    DMA2_Stream3->CR = DMA_DIR_PeripheralToMemory |
                                       DMA_Channel_4 | DMA_PeripheralInc_Disable | DMA_MemoryInc_Enable |
                                       DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
                                       DMA_Mode_Normal | DMA_Priority_VeryHigh |
                                       DMA_MemoryBurst_INC4 | DMA_PeripheralBurst_INC4 | DMA_SxCR_PFCTRL | DMA_SxCR_EN | DMA_SxCR_TCIE;
                }
            }

            /* Clear all command/response interrupts */
            SDIO->ICR = (SDIO_STA_CMDREND | SDIO_STA_CMDSENT);
        }

        /* Check whether the external interrupt was triggered */
        if ( ( intstatus & SDIO_STA_SDIOIT ) != 0 )
        {
            /* Clear the interrupt and then inform WICED thread */
            SDIO->MASK &= (~SDIO_MASK_SDIOITIE);
            SDIO->ICR = SDIO_ICR_SDIOITC;
            wwd_thread_notify_irq( );
        }
    }
}

WWD_RTOS_DEFINE_ISR( DMA2_Stream3_IRQHandler )
{
    wwd_result_t result;

    /* Clear interrupt */
    DMA2->LIFCR = (uint32_t) (0x3F << 22);

    result = host_rtos_set_semaphore( &sdio_transfer_finished_semaphore, WICED_TRUE );

    /* check result if in debug mode */
    wiced_assert( "failed to set dma semaphore", result == WWD_SUCCESS );

    /*@-noeffect@*/
    (void) result; /* ignore result if in release mode */
    /*@+noeffect@*/
}

/******************************************************
 *             IRQ Handler Mapping
 ******************************************************/

//WWD_RTOS_MAP_ISR( sdio_irq,     SDIO_IRQHandler         )
//WWD_RTOS_MAP_ISR( sdio_dma_irq, DMA2_Stream3_IRQHandler )

