/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _PNP_IMPL_H
#define _PNP_IMPL_H

#include "tx_api.h"
#include "nx_api.h"
#include "nxd_dns.h"

#define DEVICE_INFO_MANUfACTURER_PROPERTY "manufacturer"
#define DEVICE_INFO_MODEL_PROPERTY "model"
#define DEVICE_INFO_SWVERSION_PROPERTY "swVersion"
#define DEVICE_INFO_OSNAME_PROPERTY "osName"
#define DEVICE_INFO_PROCESSORARCHITECTURE_PROPERTY "processorArchitecture"
#define DEVICE_INFO_PROCESSORMANUfACTURER_PROPERTY "processorManufacturer"
#define DEVICE_INFO_TOTALSTORAGE_PROPERTY "totalStorage"
#define DEVICE_INFO_TOTAKMEMORY_PROPERTY "totalMemory"

#define MANUfACTURER_PROPERTY "MXChip"
#define MODEL_PROPERTY "AZ3166"
#define SWVERSION_PROPERTY "1.0.0"
#define OSNAME_PROPERTY "Arm Mbed OS v5.2"
#define PROCESSORARCHITECTURE_PROPERTY "Arm Cortex M4"
#define PROCESSORMANUfACTURER_PROPERTY "STMicro"
#define TOTALSTORAGE_PROPERTY 2048
#define TOTAKMEMORY_PROPERTY 256

#define HUMIDITY "humidity"
#define TEMPERATURE "temperature"
#define PRESSURE "pressure"
#define MAGNETOMETERX "magnetometerX"
#define MAGNETOMETERY "magnetometerY"
#define MAGNETOMETERZ "magnetometerZ"
#define ACCELEROMETERX "accelerometerX"
#define ACCELEROMETERY "accelerometerY"
#define ACCELEROMETERZ "accelerometerZ"
#define GYROSCOPEX "gyroscopeX"
#define GYROSCOPEY "gyroscopeY"
#define GYROSCOPEZ "gyroscopeZ"

#endif // _PNP_IMPL_H