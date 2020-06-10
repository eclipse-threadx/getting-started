project(azure_iot
    VERSION 6.0.0
    LANGUAGES C ASM
)

set(SOURCES
    azure-iot-preview/azure_iot/nx_azure_iot_hub_client.c
    azure-iot-preview/azure_iot/nx_azure_iot_provisioning_client.c
    azure-iot-preview/azure_iot/nx_azure_iot.c
)

add_library(${PROJECT_NAME} OBJECT 
    ${SOURCES})

add_library("azrtos::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} 
    PUBLIC 
        "AZ_NO_PRECONDITION_CHECKING"
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        azure-iot-preview/azure_iot
)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        az_noplatform 
        az::core 
        az::iot::common
        az::iot::hub
        az::iot::provisioning

        azrtos::nx_cloud
)