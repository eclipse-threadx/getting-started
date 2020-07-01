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
#include <string.h>
#include "platform_cmsis.h"
#include "platform_constants.h"
#include "platform_peripheral.h"
#include "platform_mcu_peripheral.h"
#include "platform_isr_interface.h"
#include "platform/wwd_platform_interface.h"
#include "wwd_platform_common.h"

void platform_gspi_bringup_slave( void )
{
    host_platform_init();
    host_platform_power_wifi( WICED_TRUE );
    host_platform_reset_wifi( WICED_TRUE );
    host_rtos_delay_milliseconds( (uint32_t) 1 );
    host_platform_reset_wifi( WICED_FALSE );
}

void platform_gspi_master_irq_init( platform_gpio_irq_callback_t handler, void* arg )
{
    /* Setup the interrupt input for WLAN_IRQ */
 #ifndef WWD_SPI_IRQ_FALLING_EDGE
     platform_gpio_init( &wifi_spi_pins[WWD_PIN_SPI_IRQ], INPUT_HIGH_IMPEDANCE );
     platform_gpio_irq_enable( &wifi_spi_pins[WWD_PIN_SPI_IRQ], IRQ_TRIGGER_RISING_EDGE, handler, arg );
 #else /* WWD_SPI_IRQ_FALLING_EDGE */
     platform_gpio_init( &wifi_spi_pins[WWD_PIN_SPI_IRQ], INPUT_PULL_UP );
     platform_gpio_irq_enable( &wifi_spi_pins[WWD_PIN_SPI_IRQ], IRQ_TRIGGER_FALLING_EDGE, handler, arg );
 #endif /* WWD_SPI_IRQ_FALLING_EDGE */

}
