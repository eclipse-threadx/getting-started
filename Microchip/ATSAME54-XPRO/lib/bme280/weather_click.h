/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef RX65N_CLOUD_KIT_SENSORS_H_
#define RX65N_CLOUD_KIT_SENSORS_H_

#include <stdint.h>

#include "src/bme280.h"

uint8_t init_weather_click(void);

int8_t read_bme280(struct bme280_data* data);

#endif
