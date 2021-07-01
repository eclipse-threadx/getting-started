/* Copyright (c) 2021 Linaro Limited. */

#include <stdio.h>

#include "tx_api.h"

#include "board_init.h"
#include "cmsis.h"
#include "tfm_api.h"
#include "tfm_ns_interface.h"
#ifdef TFM_PSA_API
#include "psa_manifest/sid.h"
#endif

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

/* TODO: Locking for accessing the TFM interface */
int32_t tfm_ns_interface_dispatch(veneer_fn fn,
                                  uint32_t arg0, uint32_t arg1,
                                  uint32_t arg2, uint32_t arg3)
{
    return fn(arg0, arg1, arg2, arg3);
}

TX_THREAD azure_thread;
ULONG azure_thread_stack[AZURE_THREAD_STACK_SIZE / sizeof(ULONG)];

void azure_thread_entry(ULONG parameter);
void tx_application_define(void* first_unused_memory);

void azure_thread_entry(ULONG parameter)
{
    uint32_t version;
    printf("Starting Azure thread\r\n");
    version = psa_framework_version();
    if (version == PSA_FRAMEWORK_VERSION) {
        printf("The version of the PSA Framework API is %lu.\n",
               version);
    } else {
        printf("The version of the PSA Framework API is not valid!\n");
        return;
    }
    psa_handle_t handle;

    handle = psa_connect(IPC_SERVICE_TEST_BASIC_SID,
                         IPC_SERVICE_TEST_BASIC_VERSION);
    if (handle > 0) {
        printf("Connect success!\n");
    } else {
        printf("The RoT Service has refused the connection!\n");
        return;
    }
    psa_close(handle);
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
