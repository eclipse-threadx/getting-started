include_guard(GLOBAL)
message("component_codec_i2c component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fsl_codec_i2c.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)


#OR Logic component
if(CONFIG_USE_component_flexcomm_i2c_adapter_MIMXRT1062)
     include(component_flexcomm_i2c_adapter_MIMXRT1062)
endif()
if(CONFIG_USE_component_i2c_adapter_MIMXRT1062)
     include(component_i2c_adapter_MIMXRT1062)
endif()
if(CONFIG_USE_component_ii2c_adapter_MIMXRT1062)
     include(component_ii2c_adapter_MIMXRT1062)
endif()
if(CONFIG_USE_component_lpc_i2c_adapter_MIMXRT1062)
     include(component_lpc_i2c_adapter_MIMXRT1062)
endif()
if(CONFIG_USE_component_lpi2c_adapter_MIMXRT1062)
     include(component_lpi2c_adapter_MIMXRT1062)
endif()
if(CONFIG_USE_component_i3c_adapter_MIMXRT1062)
     include(component_i3c_adapter_MIMXRT1062)
endif()
if(NOT (CONFIG_USE_component_flexcomm_i2c_adapter_MIMXRT1062 OR CONFIG_USE_component_i2c_adapter_MIMXRT1062 OR CONFIG_USE_component_ii2c_adapter_MIMXRT1062 OR CONFIG_USE_component_lpc_i2c_adapter_MIMXRT1062 OR CONFIG_USE_component_lpi2c_adapter_MIMXRT1062 OR CONFIG_USE_component_i3c_adapter_MIMXRT1062))
    message(WARNING "Since component_flexcomm_i2c_adapter_MIMXRT1062/component_i2c_adapter_MIMXRT1062/component_ii2c_adapter_MIMXRT1062/component_lpc_i2c_adapter_MIMXRT1062/component_lpi2c_adapter_MIMXRT1062/component_i3c_adapter_MIMXRT1062 is not included at first or config in config.cmake file, use component_i2c_adapter_MIMXRT1062 by default.")
    include(component_i2c_adapter_MIMXRT1062)
endif()

include(driver_common_MIMXRT1062)

