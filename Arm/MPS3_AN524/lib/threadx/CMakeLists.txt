# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    cmsis_rtos_v2.c
)

set(TARGET threadx-cmsis)

set(TX_CMSIS_OS2_NUM_SEMAPHORES
    "1"
    CACHE STRING
    "The CMSIS OS2 port relies on static allocations of semaphores. This configures
     the size of that static allocation in number of semaphores."
)

set(TX_CMSIS_OS2_NUM_THREADS
    "2"
    CACHE STRING
    "The CMSIS OS2 port relies on static allocations of threads. This configures
     the size of that static allocation in number of threads."
)

set(TX_CMSIS_OS2_THREAD_STACK_SIZE
    "768"
    CACHE STRING
    "The CMSIS OS2 port relies on static allocations of thread stacks. This
     configures the size of each thread's stack in bytes."
)

add_compile_definitions(TX_CMSIS_OS2_NUM_SEMAPHORES=${TX_CMSIS_OS2_NUM_SEMAPHORES})
add_compile_definitions(TX_CMSIS_OS2_NUM_THREADS=${TX_CMSIS_OS2_NUM_THREADS})
add_compile_definitions(TX_CMSIS_OS2_THREAD_STACK_SIZE=${TX_CMSIS_OS2_THREAD_STACK_SIZE})

add_library(${TARGET} OBJECT
    ${SOURCES}
)

target_include_directories(${TARGET}
    INTERFACE
        inc
)

target_link_libraries(${TARGET}
    threadx
)
