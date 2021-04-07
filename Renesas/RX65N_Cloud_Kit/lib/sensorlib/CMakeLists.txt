# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    src/bme68x/bme68x.c
    src/bmi160/bmi160.c
    src/isl29035/isl29035_sensor.c
    
    src/rx_i2c_api.c
    src/rx65n_cloud_kit_sensors.c
)

set(TARGET sensorlib)

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_include_directories(${TARGET}
    PUBLIC
        src/
        src/bme68x
)

target_link_libraries(${TARGET} 
    PUBLIC
        rx_driver_package
)
