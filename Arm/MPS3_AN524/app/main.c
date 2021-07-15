/* Copyright (c) 2021 Linaro Limited. */

#include <stdio.h>

#include "tx_api.h"
#include "tx_cmsis_os2.h"

#include "board_init.h"
#include "cmsis.h"
#include "tfm_api.h"
#include "tfm_ns_interface.h"
#ifdef TFM_PSA_API
#include "psa_manifest/sid.h"
#endif

#ifdef TFM_REGRESSION
#include "test_framework_integ_test.h"
#endif

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY   4

int tfm_log_printf(const char *fmt, ...)
{
    printf(fmt);
}

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

TX_MUTEX tfm_ns_dispatch_mutex;

int32_t tfm_ns_interface_dispatch(veneer_fn fn,
                                  uint32_t arg0, uint32_t arg1,
                                  uint32_t arg2, uint32_t arg3)
{
    int32_t ret = 0;
    tx_mutex_get(&tfm_ns_dispatch_mutex, TX_WAIT_FOREVER);
    ret = fn(arg0, arg1, arg2, arg3);
    tx_mutex_put(&tfm_ns_dispatch_mutex);
    return ret;
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

#ifdef TFM_REGRESSION
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

    /* Start TF-M NS regression tests */
    tfm_non_secure_client_run_tests();
#endif /* TFM_REGRESSION */

#ifdef TFM_PSA_TEST
    extern void val_entry(void);
    val_entry();
#endif /* TFM_PSA_TEST */
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

    tx_mutex_create(&tfm_ns_dispatch_mutex, "TF-M Dispatch Mutex", TX_INHERIT);
    init_cmsis_os2();
    tx_kernel_enter();

    return 0;
}
