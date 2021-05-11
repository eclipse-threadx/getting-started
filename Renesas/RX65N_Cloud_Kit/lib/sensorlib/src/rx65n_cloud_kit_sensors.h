/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef RX65N_CLOUD_KIT_SENSORS_H_
#define RX65N_CLOUD_KIT_SENSORS_H_

#include <stdint.h>

#include "bme68x/bme68x.h"
#include "bmi160/bmi160.h"
#include "isl29035/isl29035_sensor.h"

uint8_t init_sensors(void);

int8_t read_bme680(struct bme68x_data* data);
int8_t read_bmi160_accel(struct bmi160_sensor_data* data);
int8_t read_bmi160_gyro(struct bmi160_sensor_data* data);
int8_t read_isl29035(double* als);

#endif
