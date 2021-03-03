# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

function(post_build TARGET)
    if(THREADX_TOOLCHAIN STREQUAL "iar")
        add_custom_target(${TARGET}.bin ALL 
            DEPENDS ${TARGET}
            COMMAND ${CMAKE_IAR_ELFTOOL} --bin ${TARGET}.elf ${TARGET}.bin)
    else()
        add_custom_target(${TARGET}.bin ALL 
            DEPENDS ${TARGET}
            COMMAND ${CMAKE_OBJCOPY} -Obinary ${TARGET}.elf ${TARGET}.bin
            COMMAND ${CMAKE_OBJCOPY} -Oihex ${TARGET}.elf ${TARGET}.hex)            
    endif()
endfunction()

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()
