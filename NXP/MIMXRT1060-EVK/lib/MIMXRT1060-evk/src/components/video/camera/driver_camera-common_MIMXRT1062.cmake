include_guard(GLOBAL)
message("driver_camera-common component is included.")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)

include(driver_video-common_MIMXRT1062)

