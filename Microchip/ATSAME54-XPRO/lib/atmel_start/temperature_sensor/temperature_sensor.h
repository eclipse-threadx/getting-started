/**
 * \file
 *
 * \brief Temperature Sensor declaration.
 *
 * Copyright (c) 2016-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

#ifndef _TEMPERATURE_SENSOR_H_INCLUDED
#define _TEMPERATURE_SENSOR_H_INCLUDED

#include <compiler.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Forward declaration of temperature sensor structure. */
struct temperature_sensor;

/**
 * \brief Interface of abstract temperature sensor
 */
struct temperature_sensor_interface {
	float (*read)();
};

/**
 * \brief Abstract temperature sensor
 */
struct temperature_sensor {
	/** The pointer to interface used to communicate with temperature sensor */
	void *io;
	/** The interface of abstract temperature sensor */
	const struct temperature_sensor_interface *interface;
};

/**
 * \brief Construct abstract temperature sensor
 *
 * \param[in] me The pointer to temperature sensor to initialize
 * \param[in] io The pointer to instance of interface to actual sensor
 * \param[in] interface The pointer to interface of temperature sensor
 *
 * \return pointer to initialized sensor
 */
struct temperature_sensor *temperature_sensor_construct(struct temperature_sensor *const me, void *const io,
                                                        const struct temperature_sensor_interface *const interface);

/**
 * \brief Read temperature from the given sensor
 *
 * \param[in] me The pointer to temperature sensor to read temperature from
 *
 * \return temperature
 */
float temperature_sensor_read(const struct temperature_sensor *const me);

#ifdef __cplusplus
}
#endif

#endif /* _TEMPERATURE_SENSOR_H_INCLUDED */
