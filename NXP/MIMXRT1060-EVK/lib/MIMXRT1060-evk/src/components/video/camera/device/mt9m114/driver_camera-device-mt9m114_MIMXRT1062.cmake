include_guard(GLOBAL)
message("driver_camera-device-mt9m114 component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fsl_mt9m114.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)


include(driver_video-common_MIMXRT1062)

include(driver_camera-common_MIMXRT1062)

include(driver_camera-device-common_MIMXRT1062)

include(driver_video-i2c_MIMXRT1062)

