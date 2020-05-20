cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

# See https://cmake.org/cmake/help/latest/policy/CMP0079.html for more info
cmake_policy(SET CMP0079 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# set Security Module version
FILE(READ ${CMAKE_CURRENT_LIST_DIR}/version.txt asc_security_azurertos_VERSION)
message(STATUS "Security Module Version: ${asc_security_azurertos_VERSION}")

# Project name, version and languages
project(azure_iot_security_module
    VERSION ${asc_security_azurertos_VERSION}
    LANGUAGES C ASM
)

# Define our target library and an alias for consumers
add_library(${PROJECT_NAME})
add_library("azrtos::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    -DSECURITY_MODULE_VERSION=${asc_security_azurertos_VERSION}
    -DCOLLECTOR_SYSTEM_INFORMATION_ENABLED
    -DCOLLECTOR_CONNECTION_CREATE_ENABLED
    -DCOLLECTOR_HEARTBEAT_ENABLED
    -DASC_CORE_MAX_EVENTS_IN_MEMORY=32
    -DASC_MESSAGE_MAX_SIZE=6128
    -DASC_EVENT_MAX_SIZE=2048
    -DLOG_LEVEL=1
)

target_sources(${PROJECT_NAME}
    PRIVATE

    # Core source files
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/asc/asc_json_builder.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/asc/asc_json_parser.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/asc/asc_json_pointer.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/asc/asc_json_string.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/asc/asc_json_token.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/asc/asc_span.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/collector_collection.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/collector_collection_factory.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/collector_collection_internal.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/collectors/heartbeat.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/collectors_information.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/core.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/logger.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/message_schema_consts.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/model/collector.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/model/event.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/model/message/message_v10.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/model/schema/schema_connection_create.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/model/schema/schema_heartbeat.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/model/schema/schema_system_information.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/utils/notifier.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/src/utils/string_utils.c

    # Azure RTOS source files
    # ${CMAKE_CURRENT_LIST_DIR}/src/security_module_iothub.c
    ${CMAKE_CURRENT_LIST_DIR}/src/security_module_mqtt.c
    ${CMAKE_CURRENT_LIST_DIR}/src/collectors/connection_create.c
    ${CMAKE_CURRENT_LIST_DIR}/src/collectors/system_information.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/irand.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/itime.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/iuuid.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/network_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/os_utils.c
)

target_include_directories(${PROJECT_NAME}
    PUBLIC

    ${CMAKE_CURRENT_LIST_DIR}/azure_iot_security_core/inc
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# Define any required dependencies between this library and others
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        azrtos::nx_cloud
        azrtos::threadx
        azrtos::netxduo
)

