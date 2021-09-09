include_guard(GLOBAL)
message("driver_phy-common component is included.")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)

include(driver_mdio-common_MIMXRT1062)

