/* Copyright (c) 2021 Linaro Limited. */

#include <stdio.h>

#include "tx_api.h"

#include "board_init.h"
#include "cmsis.h"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY   4

static __inline void systick_interval_set(uint32_t ticks)
{
    // 1. Disable the counter
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    // 2. Update the Systick timer period
    SysTick->LOAD = SystemCoreClock / ticks - 1;

    // 3. Clear the current value
    SysTick->VAL  = 0;

    // 4. Enable the counter
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

TX_THREAD azure_thread;
ULONG azure_thread_stack[AZURE_THREAD_STACK_SIZE / sizeof(ULONG)];

void azure_thread_entry(ULONG parameter);
void tx_application_define(void* first_unused_memory);

void azure_thread_entry(ULONG parameter)
{
    printf("\r\nStarting Azure thread\r\n\r\n");
}

void tx_application_define(void* first_unused_memory)
{
    systick_interval_set(TX_TIMER_TICKS_PER_SECOND);

    // Create Azure thread
    UINT status = tx_thread_create(&azure_thread,
        "Azure Thread",
        azure_thread_entry,
        0,
        azure_thread_stack,
        AZURE_THREAD_STACK_SIZE,
        AZURE_THREAD_PRIORITY,
        AZURE_THREAD_PRIORITY,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);

    if (status != TX_SUCCESS)
    {
        printf("Azure IoT thread creation failed\r\n");
    }
}

int main(void)
{
    /* Initialise the board */
    board_init();

    printf("Azure RTOS running on MPS3 board\n");

    
    tx_kernel_enter();

    return 0;
}
