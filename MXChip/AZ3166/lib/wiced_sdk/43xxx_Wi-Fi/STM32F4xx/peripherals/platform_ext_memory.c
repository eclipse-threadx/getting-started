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
#include <stdint.h>
#include "wwd_constants.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "platform_ext_memory.h"
#include "platform_external_memory.h"
#include <string.h>
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define BCM9WCD1AUDIO

/******************************************************
 *                      Macros
 ******************************************************/
#define ARRAY_SIZE(a)         ( sizeof(a) / sizeof(a[0]) )

/******************************************************
 *                    Constants
 ******************************************************/
#define CHIP_SELECT_AREA_SIZE ( 0x4000000 ) /* 64 Mbytes for every chip select */

/* Chip select control registers */
#define FSMC_BCR1           ((volatile uint32_t*)(0xA0000000))
#define FSMC_BCR2           ((volatile uint32_t*)(0xA0000008))
#define FSMC_BCR3           ((volatile uint32_t*)(0xA0000010))
#define FSMC_BCR4           ((volatile uint32_t*)(0xA0000018))

/* Chip select timing */
#define FSMC_BTCR1          ((volatile uint32_t*)(0xA0000004))
#define FSMC_BTCR2          ((volatile uint32_t*)(0xA000000C))
#define FSMC_BTCR3          ((volatile uint32_t*)(0xA0000014))
#define FSMC_BTCR4          ((volatile uint32_t*)(0xA000001C))

/* Write timings for extended mode */
#define FSMC_BWTR1          ((volatile uint32_t*)(0xA0000104))
#define FSMC_BWTR2          ((volatile uint32_t*)(0xA000010C))
#define FSMC_BWTR3          ((volatile uint32_t*)(0xA0000114))
#define FSMC_BWTR4          ((volatile uint32_t*)(0xA000011C))

#define SRAM_BANK_ID        (0)
#define NAND2_BANKID        (1)
#define NAND1_BANKID        (2)
#define CLK_CLOCK           (1)
#define HCLK_CLOCK          (2)
#define MIN_DATLAT_CYCLES   (2)
#define MAX_DATLAT_CYCLES   (17)
#define MIN_BUSTURN_CYCLES  (0)
#define MAX_BUSTURN_CYCLES  (15)
#define MIN_DATAST_CYCLES   (1)
#define MAX_DATAST_CYCLES   (255)
#define MIN_ADDHLD_CYCLES   (1)
#define MAX_ADDHLD_CYCLES   (255)
#define MIN_ADDSET_CYCLES   (0)
#define MAX_ADDSET_CYCLES   (1615)


/* register fields - BCR */
#define CBURSTRW_START      (19)
#define ASYNCWAIT_START     (15)
#define EXTMOD_START        (14)
#define WAITEN_START        (13)
#define WAITEN_START        (13)
#define WREN_START          (12)
#define WAITCFG_START       (11)
#define WRAPMOD_START       (10)
#define WAITPOL_START       (9)
#define BURSTEN_START       (8)
#define FACCEN_START      (6)
#define MWID_START        (4)
#define MTYP_START        (2)
#define MUXEN_START       (1)
#define MBKEN_START       (0)

/* register fields - BTR, general timing settings */
#define ACCMOD_START      (28)
#define DATLAT_START      (24)
#define CLKDIV_START      (20)
#define BUSTURN_START     (16)
#define DATAST_START      (8)
#define ADDHLD_START      (4)
#define ADDSET_START      (0)

/* register fields BWTR, timing settings for write transactions */
#define ACCMOD_WR_START   (28)
#define DATLAT_WR_START   (24)
#define CLKDIV_WR_START   (20)
#define BUSTURN_WR_START  (16)
#define DATAST_WR_START   (8)
#define ADDHLD_WR_START   (4)
#define ADDSET_WR_START   (0)

/******************************************************
 *                   Enumerations
 ******************************************************/
typedef enum
{
    GPIOA_index,
    GPIOB_index,
    GPIOC_index,
    GPIOD_index,
    GPIOE_index,
    GPIOF_index,
    GPIOG_index,
    GPIOH_index,
    GPIOI_index,
} pin_registerer_indexes;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
typedef struct
{
    /* gpio related data */
    GPIO_TypeDef* gpio_port;
    pin_registerer_indexes index;
    uint16_t      gpio_pin;
} pin_info_t;

typedef struct
{
    GPIO_TypeDef * port;
    uint32_t       pins;
} port_utilization_t;

typedef struct
{
    volatile uint32_t* bcr;
    volatile uint32_t* btcr;
    volatile uint32_t* bwtr;
} stm32f4_sram_nor_control_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

int fsmc_sram_configure( const wiced_sram_device_t* sram, const stm32f4xx_platform_nor_sram_t* settings );
int f4_sram_timings_init ( const wiced_sram_device_t* sram, const stm32f4xx_platform_nor_sram_t* settings );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Only four chip select pins are available NE1-NE4 */
pin_info_t chip_select_pins[4]=
{
        {
                /* Pin NE1 */
                .gpio_port = GPIOD,
                .index = GPIOD_index,
                .gpio_pin = 7
        },
        {
                /* Pin NE2 */
                .gpio_port = GPIOG,
                .index = GPIOG_index,
                .gpio_pin = 9

        },
        {
                /* Pin NE3 */
                .gpio_port = GPIOG,
                .index = GPIOG_index,
                .gpio_pin = 10
        },
        {
                /* Pin NE4 */
                .gpio_port = GPIOG,
                .index = GPIOG_index,
                .gpio_pin = 12
        },
};


const stm32f4_sram_nor_control_t f2_sram_nor_control_regs[] =
{

        /* NE1 chip select control registers */
        {
                .bcr    = FSMC_BCR1,
                .btcr   = FSMC_BTCR1,
                .bwtr   = FSMC_BWTR1,
        },

        /* NE2 chip select control registers */
        {
                .bcr    = FSMC_BCR2,
                .btcr   = FSMC_BTCR2,
                .bwtr   = FSMC_BWTR2,
        },
        /* NE3 chip select */
        {
                .bcr    = FSMC_BCR3,
                .btcr   = FSMC_BTCR3,
                .bwtr   = FSMC_BWTR3,
        },
        /* NE4 chip select */
        {
                .bcr    = FSMC_BCR4,
                .btcr   = FSMC_BTCR4,
                .bwtr   = FSMC_BWTR4,
        }
};


static int ns_to_cycles( unsigned int num_ns, int* cycles, int max, int min, int clock_type )
{
    UNUSED_PARAMETER(max);
    UNUSED_PARAMETER(min);
    UNUSED_PARAMETER(clock_type);

    /* Should be fixed */
    *cycles = (int)num_ns;
    return 0;
}

int f4_sram_timings_init ( const wiced_sram_device_t* sram, const stm32f4xx_platform_nor_sram_t* settings )
{
    int retval = 0;
    int temp;
    const stm32f4_sram_nor_control_t* regs = &f2_sram_nor_control_regs[sram->chip_select];

    /* WAICFG bit is untouched currently */
    ( settings->wr_burst            == 1 ) ? ( *regs->bcr |= ( uint32_t )( 1 << CBURSTRW_START ) )  : ( *regs->bcr &= ( uint32_t )(~( 1 << CBURSTRW_START )) );
    ( settings->async_wait_enabled  == 1 ) ? ( *regs->bcr |= ( uint32_t )( 1 << ASYNCWAIT_START ) ) : ( *regs->bcr &= ( uint32_t )(~( 1 << ASYNCWAIT_START )) );
    ( settings->extended_mode       == 1 ) ? ( *regs->bcr |= ( uint32_t )( 1 << EXTMOD_START ) )    : ( *regs->bcr &= ( uint32_t )(~( 1 << EXTMOD_START )) );
    ( settings->sync_wait_enabled   == 1 ) ? ( *regs->bcr |= ( uint32_t )( 1 << WAITEN_START ) )    : ( *regs->bcr &= ( uint32_t )(~( 1 << WAITEN_START )) );
    ( settings->wait_polarity       == 1 ) ? ( *regs->bcr |= ( uint32_t )( 1 << WAITPOL_START ) )   : ( *regs->bcr &= ( uint32_t )(~( 1 << WAITPOL_START )) );
    ( settings->rd_burst            == 1 ) ? ( *regs->bcr |= ( uint32_t )( 1 << BURSTEN_START ) )   : ( *regs->bcr &= ( uint32_t )(~( 1 << BURSTEN_START )) );

    /* This is nor a nor flash */
    *regs->bcr &= ( uint32_t )( ~( 1 << FACCEN_START ) );
    /* Clear MTYP, must be 0 */
    *regs->bcr &= ( uint32_t )( ~( 0x03 << MTYP_START ) );

    /* Select access mode */
    *regs->btcr &= ( 0x03 << ACCMOD_START ); /* !!!! must be bit-wise inversion */
    *regs->btcr |= ( uint32_t )( settings->access_mode << ACCMOD_START );
    /* Select data-latency */
    retval = ns_to_cycles( settings->data_latency, &temp, MAX_DATLAT_CYCLES, MIN_DATLAT_CYCLES, CLK_CLOCK );
    if( retval != 0 )
    {
        return retval;
    }
    /* Select clk divider for synchronous memories  */
    /* TODO */

    /* Select bus-turnaround duration */
    retval = ns_to_cycles( settings->bus_turnaround, &temp, MAX_BUSTURN_CYCLES, MIN_BUSTURN_CYCLES, HCLK_CLOCK );
    if( retval != 0 )
    {
        return retval;
    }
    *regs->btcr &= ( uint32_t )( ~( 0x0F << BUSTURN_START ) );
    *regs->btcr |= ( uint32_t )( ( temp << BUSTURN_START ) );

    /* Select data phase duration */
    retval = ns_to_cycles( settings->data_phase_duration, &temp, MAX_DATAST_CYCLES, MIN_DATAST_CYCLES, HCLK_CLOCK );
    if( retval != 0 )
    {
        return retval;
    }
    /* 8 bits are used for DATAST duration field */
    *regs->btcr &= ( uint32_t )( ~( 0xFF << DATAST_START ) );
    *regs->btcr |= ( uint32_t )( temp << DATAST_START );

    /* Select address hold duration */
    retval = ns_to_cycles( settings->address_hold_duration, &temp, MAX_ADDHLD_CYCLES, MIN_ADDHLD_CYCLES, HCLK_CLOCK );
    if( retval != 0 )
    {
        return retval;
    }
    *regs->btcr &= ( uint32_t )( ~( 0x0F << ADDHLD_START ) );
    *regs->btcr |= ( uint32_t )( temp << ADDHLD_START );

    /* Select address setup phase duration */
    retval = ns_to_cycles( settings->address_set_duration, &temp, MAX_ADDSET_CYCLES, MIN_ADDSET_CYCLES, HCLK_CLOCK );
    /* Check whether the addset duration will be in the available HCL range */
    if( retval != 0 )
    {
        return retval;
    }
    if( temp >= 15 )
    {
        /* Set to maximum available */
        temp = 15;
    }
    *regs->btcr &= ( uint32_t )( ~( 0x0F << ADDSET_START ) );
    *regs->btcr |= ( uint32_t )( temp << ADDSET_START );

    /* in extended mode, write memory transactions timings are different from read memory transactions timings */

    if( settings->extended_mode == 1 )
    {
        /* Select access mode */
        *regs->bwtr &= ( 0x03 << ACCMOD_START );
        *regs->bwtr |= ( uint32_t )( ( settings->wr_access_mode << ACCMOD_START ) );
        /* Select data-latency */
        retval = ns_to_cycles( settings->wr_data_latency, &temp, MAX_DATLAT_CYCLES, MIN_DATLAT_CYCLES, CLK_CLOCK );
        if( retval != 0 )
        {
            return retval;
        }
        /* Select clk divider for synchronous memories  */
        /* ???????? */

        /* Select bus-turnaround duration */
        retval = ns_to_cycles( settings->wr_bus_turnaround, &temp, MAX_BUSTURN_CYCLES, MIN_BUSTURN_CYCLES, HCLK_CLOCK );
        if( retval != 0 )
        {
            return retval;
        }
        *regs->bwtr &= ( uint32_t )( ~( 0x0F << BUSTURN_START ) );
        *regs->bwtr |= ( uint32_t )( temp << BUSTURN_START );

        /* Select data phase duration */
        retval = ns_to_cycles( settings->wr_data_phase_duration, &temp, MAX_DATAST_CYCLES, MIN_DATAST_CYCLES, HCLK_CLOCK );
        if( retval != 0 )
        {
            return retval;
        }
        /* 8 bits are used for DATAST duration field */
        *regs->bwtr &= ( uint32_t )( ~( 0xFF << DATAST_START ) );
        *regs->bwtr |= ( uint32_t )( temp << DATAST_START );

        /* Select address hold duration */
        retval = ns_to_cycles( settings->wr_address_hold_duration, &temp, MAX_ADDHLD_CYCLES, MIN_ADDHLD_CYCLES, HCLK_CLOCK );
        if( retval != 0 )
        {
            return retval;
        }
        *regs->bwtr &= ( uint32_t )( ~( 0x0F << ADDHLD_START ) );
        *regs->bwtr |= ( uint32_t )( temp << ADDHLD_START );

        /* Select address setup phase duration */
        retval = ns_to_cycles( settings->wr_address_set_duration, &temp, MAX_ADDSET_CYCLES, MIN_ADDSET_CYCLES, HCLK_CLOCK );
        /* get the ratio */
        retval = ( temp * 1615 ) / 15;
        if( retval != 0 )
        {
            return retval;
        }
        *regs->bwtr &= ( uint32_t )( ~( 0x0F << ADDSET_START ) );
        *regs->bwtr |= ( uint32_t )( temp << ADDSET_START );
    }

    return 0;
}

#if 0
int fsmc_sram_configure( const wiced_sram_device_t* sram, const stm32f4xx_platform_nor_sram_t* settings )
{
    int retval;
    const stm32f4_sram_nor_control_t* regs = &f2_sram_nor_control_regs[sram->chip_select];

    /* Enable FSMC clock */
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

    /* Clear and then set bus data width */
    *( regs->bcr ) &= (uint32_t)( ~( 0x03 << MWID_START ) );
    ( sram->bus_data_width == 16 ) ? ( *( regs->bcr ) |= ( uint32_t )( 1 << MWID_START ) ) : ( *( regs->bcr ) &= ( uint32_t )( ~( 1 << MWID_START ) ) ) ;

    /* enable writes on this memory device. IT is RAM */
    *(regs->bcr) |= ( uint32_t )( 1 << WREN_START );
    /* check whether address lines are multiplexed with data lines and enable must functionality */
    ( sram->mux_addr_data == 1 ) ? ( *(regs->bcr) |= ( 1 << MUXEN_START ) ) : ( *(regs->bcr) &= ( uint32_t )( ~( 1 << MUXEN_START ) ) );

    retval = f4_sram_timings_init( sram, settings );
    if ( retval != 0 )
    {
         goto error_handling;
    }
    f4_sram_pins_init( sram, settings );

    /* enable this memory bank, nor a chip select( for every memory bank we can have 4 chip selects ) */
    /* But still remember that they are all shared between all 4 banks. Once used for one of the banks */
    /* chip select can not be used again */
    *(regs->bcr) |= ( 1 << MBKEN_START );

    return 0;

error_handling:

    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, DISABLE);
    return retval;
}


static int f4_nor_configure( wiced_nor_device_t* nor, stm32f4xx_platform_nor_sram_t* settings )
{
    UNUSED_PARAMETER(nor);
    UNUSED_PARAMETER(settings);

    return 0;

}

static int f4_nand_configure( wiced_nand_device_t* nand, stm32f4xx_platform_nor_sram_t* settings )
{
    UNUSED_PARAMETER(nand);
    UNUSED_PARAMETER(settings);

    return WICED_UNSUPPORTED;
}
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif
