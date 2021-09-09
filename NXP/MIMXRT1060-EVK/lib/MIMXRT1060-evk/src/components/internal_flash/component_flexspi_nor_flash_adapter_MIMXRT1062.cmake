include_guard(GLOBAL)
message("component_flexspi_nor_flash_adapter component is included.")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)

#OR Logic component
if(CONFIG_USE_component_flexspi_nor_flash_adapter_rt1060evk_MIMXRT1062)
     include(component_flexspi_nor_flash_adapter_rt1060evk_MIMXRT1062)
endif()
if(CONFIG_USE_component_flexspi_nor_flash_adapter_rt685evk_MIMXRT1062)
     include(component_flexspi_nor_flash_adapter_rt685evk_MIMXRT1062)
endif()
if(NOT (CONFIG_USE_component_flexspi_nor_flash_adapter_rt1060evk_MIMXRT1062 OR CONFIG_USE_component_flexspi_nor_flash_adapter_rt685evk_MIMXRT1062))
    message(WARNING "Since component_flexspi_nor_flash_adapter_rt1060evk_MIMXRT1062/component_flexspi_nor_flash_adapter_rt685evk_MIMXRT1062 is not included at first or config in config.cmake file, use component_flexspi_nor_flash_adapter_rt1060evk_MIMXRT1062/component_flexspi_nor_flash_adapter_rt1060evk_support_MIMXRT1062 by default.")
    include(component_flexspi_nor_flash_adapter_rt1060evk_MIMXRT1062)
    include(component_flexspi_nor_flash_adapter_rt1060evk_support_MIMXRT1062)
endif()

