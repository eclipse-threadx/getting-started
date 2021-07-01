# Copyright (c) 2021 Linaro Limited.
# Licensed under the MIT License.

set(APP_CONFIG_OPTIONS "${CMAKE_CURRENT_LIST_DIR}/config.cmake" CACHE FILEPATH
    "Configuration Options"
)

include(${APP_CONFIG_OPTIONS})

set(LINKER_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/startup/mps3_an524.ld")

set(SOURCES
    startup/startup_cmsdk_mps3_an524_bl2.S
    main.c
    board_init.c
    console.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        azrtos::threadx

        # app_common
        # jsmn
        AN524
        tfm_api
        # netx_driver
)

target_link_options(${PROJECT_NAME}
    PRIVATE 
        -T${LINKER_SCRIPT} -Wl,-Map=${PROJECT_NAME}.map)

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        LINK_DEPENDS ${LINKER_SCRIPT}
        SUFFIX ".elf"
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
)

if(BUILD_WITH_TFM)
    if (NOT TFM_MCUBOOT_IMAGE_NUMBER STREQUAL "OFF")
       set(TFM_MCUBOOT_IMAGE_NUMBER_OPTION "MCUBOOT_IMAGE_NUMBER ${TFM_MCUBOOT_IMAGE_NUMBER}")
    endif()
    if (TFM_ISOLATION_LEVEL)
       set(TFM_ISOLATION_LEVEL_OPTION "ISOLATION_LEVEL ${TFM_ISOLATION_LEVEL}")
    endif()
    if (TFM_KEY_FILE_S)
       set(TFM_KEY_FILE_S_OPTION "KEY_FILE_S ${TFM_KEY_FILE_S}")
    endif()
    if (TFM_KEY_FILE_NS)
       set(TFM_KEY_FILE_NS_OPTION "KEY_FILE_NS ${TFM_KEY_FILE_NS}")
    endif()
    if (TFM_PROFILE)
       set(TFM_PROFILE_OPTION BUILD_PROFILE ${TFM_PROFILE})
    endif()
    if (TFM_BL2)
       set(TFM_BL2_OPTION "BL2")
    endif()
    if (TFM_IPC)
       set(TFM_IPC_OPTION "IPC")
    endif()
    if (TFM_REGRESSION)
       set(TFM_REGRESSION_OPTION "REGRESSION")
    endif()
    if (TFM_PARTITION_PROTECTED_STORAGE)
       list(APPEND PARTITIONS "TFM_PARTITION_PROTECTED_STORAGE")
    endif()
    if (TFM_PARTITION_INTERNAL_TRUSTED_STORAGE)
       list(APPEND PARTITIONS "TFM_PARTITION_INTERNAL_TRUSTED_STORAGE")
    endif()
    if (TFM_PARTITION_PLATFORM)
       list(APPEND PARTITIONS "TFM_PARTITION_PLATFORM")
    endif()
    if (TFM_PARTITION_CRYPTO)
       list(APPEND PARTITIONS "TFM_PARTITION_CRYPTO")
    endif()
    if (TFM_PARTITION_INITIAL_ATTESTATION)
       list(APPEND PARTITIONS "TFM_PARTITION_INITIAL_ATTESTATION")
    endif()
    if (TFM_PARTITION_AUDIT_LOG)
       list(APPEND PARTITIONS "TFM_PARTITION_AUDIT_LOG")
    endif()
    if (TFM_PARTITION_FIRMWARE_UPDATE)
       list(APPEND PARTITIONS "TFM_PARTITION_FIRMWARE_UPDATE")
    endif()
    if (PARTITIONS)
       set(PARTITIONS_OPTION "ENABLED_PARTITIONS ${PARTITIONS}")
    endif()
    trusted_firmware_build(
        BINARY_DIR ${CMAKE_BINARY_DIR}/tfm
        BOARD arm/mps3/an524
        ${TFM_PROFILE_OPTION}
        ${TFM_MCUBOOT_IMAGE_NUMBER_OPTION}
        ${TFM_ISOLATION_LEVEL_OPTION}
        ${TFM_KEY_FILE_S_OPTION}
        ${TFM_KEY_FILE_NS_OPTION}
        ${TFM_BL2_OPTION}
        ${TFM_IPC_OPTION}
        ${TFM_REGRESSION_OPTION}
        ${PARTITIONS_OPTION}
    )
endif()
