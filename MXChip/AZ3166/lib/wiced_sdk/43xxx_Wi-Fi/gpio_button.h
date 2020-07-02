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

/**
 * @file
 *
 * GPIO-based Button APIs
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum {
    GPIO_BUTTON_OPEN = 0,
    GPIO_BUTTON_PRESSED,
} gpio_button_state_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/* These values are defined on a per-platform, per-button basis in platforms/<platform_name>/platform.c */
typedef struct
{
    wiced_gpio_t                gpio;       /* WICED_GPIO_XX value - gets translated to Board GPIO_X/PWMM_X */
    platform_pin_config_t       config;     /* INPUT_HIGH_IMPEDANCE | INPUT_PULL_UP | INPUT_PULL_DOWN       */
    wiced_active_state_t        polarity;   /* WICED_ACTIVE_LOW | WICED_ACTIVE_HIGH                         */
    wiced_gpio_irq_trigger_t    trigger;    /* IRQ trigger IRQ_TRIGGER_XXX  EDGE/LEVEL                      */
} gpio_button_t;

/* Button Change Callback function
 * Called based on the trigger value set in the button structure
 *
 * @param [in] button    : Pointer to a gpio_button_t structure associated with the IRQ
 * @param [in] new_state : New button state
 *                         WICED_TRUE  == button pressed
 *                         WICED_FALSE == button NOT pressed
 */
typedef void (*gpio_button_state_change_callback_t)( gpio_button_t* button, wiced_bool_t new_state );

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t gpio_button_init     ( const gpio_button_t* button );
wiced_result_t gpio_button_deinit   ( const gpio_button_t* button );
wiced_result_t gpio_button_enable   ( const gpio_button_t* button );
wiced_result_t gpio_button_disable  ( const gpio_button_t* button );
wiced_bool_t   gpio_button_get_value( const gpio_button_t* button );
wiced_result_t gpio_button_register_state_change_callback( gpio_button_state_change_callback_t callback );

#ifdef __cplusplus
} /*extern "C" */
#endif
