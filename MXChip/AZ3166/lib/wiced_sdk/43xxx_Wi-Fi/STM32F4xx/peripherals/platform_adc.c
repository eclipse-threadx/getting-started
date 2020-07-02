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
#include "platform_peripheral.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "wiced_utilities.h"
#include "wwd_assert.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_NUM_CHANNELS    ( 19 )

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

static const uint16_t adc_sampling_cycle[] =
{
    [ADC_SampleTime_3Cycles  ] = 3,
    [ADC_SampleTime_15Cycles ] = 15,
    [ADC_SampleTime_28Cycles ] = 28,
    [ADC_SampleTime_56Cycles ] = 56,
    [ADC_SampleTime_84Cycles ] = 84,
    [ADC_SampleTime_112Cycles] = 112,
    [ADC_SampleTime_144Cycles] = 144,
    [ADC_SampleTime_480Cycles] = 480,
};

/* locally store the sample time for use */
static uint32_t sample_cycles[MAX_NUM_CHANNELS];

/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_adc_init( const platform_adc_t* adc, uint32_t sample_cycle )
{
    GPIO_InitTypeDef       gpio_init_structure;
    ADC_InitTypeDef        adc_init_structure;
    ADC_CommonInitTypeDef  adc_common_init_structure;

    wiced_assert( "bad argument", ( adc != NULL ) );

    platform_mcu_powersave_disable();

    /* Initialize the associated GPIO */
    gpio_init_structure.GPIO_Speed = (GPIOSpeed_TypeDef) 0;
    gpio_init_structure.GPIO_Mode  = GPIO_Mode_AN;
    gpio_init_structure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init_structure.GPIO_OType = GPIO_OType_OD;
    gpio_init_structure.GPIO_Pin   = (uint32_t)( 1 << adc->pin->pin_number );
    GPIO_Init( adc->pin->port, &gpio_init_structure );

    /* Ensure the ADC and GPIOA are enabled */
    RCC_APB2PeriphClockCmd( adc->adc_peripheral_clock, ENABLE );

    /* Initialize the ADC */
    ADC_StructInit( &adc_init_structure );
    adc_init_structure.ADC_Resolution         = ADC_Resolution_12b;
    adc_init_structure.ADC_ScanConvMode       = DISABLE;
    adc_init_structure.ADC_ContinuousConvMode = DISABLE;
    adc_init_structure.ADC_ExternalTrigConv   = ADC_ExternalTrigConvEdge_None;
    adc_init_structure.ADC_DataAlign          = ADC_DataAlign_Right;
    adc_init_structure.ADC_NbrOfConversion    = 1;
    ADC_Init( adc->port, &adc_init_structure );

    ADC_CommonStructInit( &adc_common_init_structure );
    adc_common_init_structure.ADC_Mode             = ADC_Mode_Independent;
    adc_common_init_structure.ADC_DMAAccessMode    = ADC_DMAAccessMode_Disabled;
    adc_common_init_structure.ADC_Prescaler        = ADC_Prescaler_Div2;
    adc_common_init_structure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit( &adc_common_init_structure );

    ADC_Cmd( adc->port, ENABLE );

    sample_cycles[adc->channel] = sample_cycle;

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_adc_deinit( const platform_adc_t* adc )
{
    UNUSED_PARAMETER( adc );
    wiced_assert( "unimplemented", 0!=0 );
    return PLATFORM_UNSUPPORTED;
}

platform_result_t platform_adc_take_sample( const platform_adc_t* adc, uint16_t* output )
{
    wiced_assert( "bad argument", ( adc != NULL ) && ( output != NULL ) );
    uint8_t a;

    platform_mcu_powersave_disable();

    /* Find the closest supported sampling time by the MCU */
    for ( a = 0; ( a < ARRAY_SIZE(adc_sampling_cycle) ) && adc_sampling_cycle[a] < sample_cycles[adc->channel]; a++ )
    {
    }

    /*
     * Initialize the ADC channel
     * This peripheral library is not designed to handle multiple active channels per ADC port.
     *
     * Program the Rank 1 slot with desired channel, which limits the number of conversions
     * to 1 & the ADC conversion takes place only for the desired channel
     */
    ADC_RegularChannelConfig( adc->port, adc->channel, 1, a );

    /* Start conversion */
    ADC_SoftwareStartConv( adc->port );

    /* Wait until end of conversion */
    while ( ADC_GetFlagStatus( adc->port, ADC_FLAG_EOC ) == RESET )
    {
    }

    /* Read ADC conversion result */
    *output = ADC_GetConversionValue( adc->port );

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_adc_take_sample_stream( const platform_adc_t* adc, void* buffer, uint16_t buffer_length )
{
    UNUSED_PARAMETER( adc );
    UNUSED_PARAMETER( buffer );
    UNUSED_PARAMETER( buffer_length );
    wiced_assert( "unimplemented", 0!=0 );
    return PLATFORM_SUCCESS;
}
