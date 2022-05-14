set(TARGET wiced_sdk)

add_library(${TARGET}
    STATIC IMPORTED GLOBAL
)

set_target_properties(${TARGET}
    PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/libwiced_sdk_bin.a
)

target_include_directories(${TARGET}
    INTERFACE
        inc
)
