# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Create the bin output
function(create_bin_output TARGET)
    add_custom_target(${TARGET}.bin ALL 
        DEPENDS ${TARGET}
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${TARGET}.elf ${TARGET}.bin)
endfunction()

# Add custom command to print firmware size in Berkley format
function(firmware_size TARGET)
    add_custom_target(${TARGET}.size ALL 
        DEPENDS ${TARGET} 
        COMMAND ${CMAKE_SIZE_UTIL} -B ${TARGET}.elf)
endfunction()

# Output size of symbols in the resulting elf
function(symbol_size TARGET)
    add_custom_target(${TARGET}.nm ALL
        DEPENDS ${TARGET}
        COMMAND ${CMAKE_NM_UTIL} --print-size --size-sort --radix=d ${TARGET}.elf)
endfunction()
