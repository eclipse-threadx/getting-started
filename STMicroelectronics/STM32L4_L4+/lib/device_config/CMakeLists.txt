# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    device_config.C
)

set(TARGET device_config)

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_link_libraries(${TARGET}
    PUBLIC
        azrtos::threadx
        azrtos::netxduo
        stm32cubel4
)
