#include <stdbool.h>
#include <stdint.h>
#include <tx_api.h>
#include "inc/tx_cmsis_os2.h"
#include "../../../../core/lib/tf-m-tests/CMSIS/RTOS2/Include/cmsis_os2.h"

#ifndef TX_CMSIS_OS2_NUM_SEMAPHORES
#define TX_CMSIS_OS2_NUM_SEMAPHORES (1)
#endif
TX_BLOCK_POOL semaphore_pool = {0};
uint8_t semaphore_blocks[
	TX_CMSIS_OS2_NUM_SEMAPHORES * (sizeof(TX_SEMAPHORE) + sizeof(void *))
] = {0};

#ifndef TX_CMSIS_OS2_NUM_THREADS
#define TX_CMSIS_OS2_NUM_THREADS (2)
#endif
#ifndef TX_CMSIS_OS2_THREAD_STACK_SIZE
#define TX_CMSIS_OS2_THREAD_STACK_SIZE (512 + 256)
#endif
TX_BLOCK_POOL thread_pool = {0};
uint8_t thread_blocks[
	TX_CMSIS_OS2_NUM_THREADS * (sizeof(TX_THREAD) + sizeof(void *))
] = {0};

TX_BYTE_POOL thread_stack_pool = {0};
uint8_t thread_stack_blocks[
	TX_CMSIS_OS2_NUM_THREADS *
	(TX_CMSIS_OS2_THREAD_STACK_SIZE + sizeof(void *))
] = {0};

void init_cmsis_os2(void) {
	tx_block_pool_create(
		&semaphore_pool, /* pool ptr */
		"CMSIS OS2 Semaphore Pool", /* name */
		sizeof(TX_SEMAPHORE), /* block size */
		&semaphore_blocks[0], /* pool start */
		sizeof(semaphore_blocks) /* pool size */
	);
	tx_block_pool_create(
		&thread_pool, /* pool ptr */
		"CMSIS OS2 Thread Pool", /* name */
		sizeof(TX_THREAD), /* block size */
		&thread_blocks[0], /* pool start */
		sizeof(thread_blocks) /* pool size */
	);
	tx_byte_pool_create(
		&thread_stack_pool, /* pool ptr */
		"CMSIS OS2 Thread Stack Pool", /* name */
		&thread_stack_blocks[0], /* pool start */
		sizeof(thread_stack_blocks) /* pool size */
	);
}

osMutexId_t osMutexNew(const osMutexAttr_t *attr) {
	return NULL;
}

osSemaphoreId_t osSemaphoreNew (
	uint32_t max_count,
	uint32_t initial_count,
	const osSemaphoreAttr_t *attr
) {
	TX_SEMAPHORE *new_semaphore = (TX_SEMAPHORE*) attr->cb_mem;
	if (new_semaphore == NULL) {
		tx_block_allocate(
			&semaphore_pool,
			(VOID **) &new_semaphore,
			TX_WAIT_FOREVER
		);
	}
	tx_semaphore_create(new_semaphore, (CHAR *) attr->name, initial_count);
	return (osSemaphoreId_t) new_semaphore;
}

const char *osSemaphoreGetName (osSemaphoreId_t semaphore_id) {
	CHAR* name = NULL;
	tx_semaphore_info_get(
		(TX_SEMAPHORE *) semaphore_id,
		&name, /* name */
		NULL, /* current_value */
		NULL, /* first_suspended */
		NULL, /* suspend_count */
		NULL /* next_semaphore */
	);
	return name;
}

uint32_t osSemaphoreGetCount (osSemaphoreId_t semaphore_id) {
	ULONG current_count = 0;
	tx_semaphore_info_get(
		(TX_SEMAPHORE *) semaphore_id,
		NULL, /* name */
		&current_count, /* current_value */
		NULL, /* first_suspended */
		NULL, /* suspend_count */
		NULL /* next_semaphore */
	);
	return current_count;
}

osStatus_t osSemaphoreAcquire (osSemaphoreId_t semaphore_id, uint32_t timeout) {
	return tx_semaphore_get((TX_SEMAPHORE *) semaphore_id, (ULONG) timeout);
}

osStatus_t osSemaphoreRelease (osSemaphoreId_t semaphore_id) {
	return tx_semaphore_put((TX_SEMAPHORE *) semaphore_id);
}

osStatus_t osSemaphoreDelete (osSemaphoreId_t semaphore_id) {
	tx_semaphore_delete((TX_SEMAPHORE *) semaphore_id);
	return tx_block_release((TX_SEMAPHORE *) semaphore_id);
}

osThreadId_t osThreadGetId (void) {
	return (osThreadId_t) tx_thread_identify();
}

void entry_exit_notify(TX_THREAD * thread, UINT entry_exit) {
	if (entry_exit == TX_THREAD_EXIT) {
		tx_thread_delete(thread);
		tx_byte_release(thread->tx_thread_stack_start);
		tx_block_release(thread);
	}
}

osThreadId_t osThreadNew (
	osThreadFunc_t func,
	void *argument,
	const osThreadAttr_t *attr
) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wcast-function-type"
	VOID (*localfunc)(ULONG) = (VOID (*)(ULONG)) func;
	#pragma GCC diagnostic pop
	TX_THREAD *new_thread = (TX_THREAD*) attr->cb_mem;
	if (new_thread == NULL) {
		tx_block_allocate(
			&thread_pool,
			(VOID **) &new_thread,
			TX_WAIT_FOREVER
		);
	}
	VOID *stack_mem = (VOID *) attr->stack_mem;
	if (stack_mem == NULL) {
		tx_byte_allocate(
			&thread_stack_pool,
			(VOID **) &stack_mem,
			attr->stack_size,
			TX_WAIT_FOREVER
		);
	}
	tx_thread_create(
		new_thread, /* thread ptr */
		(CHAR *) attr->name, /* name */
		localfunc, /* entry function */
		(ULONG) argument, /* entry intput */
		stack_mem, /* stack start */
		(ULONG) attr->stack_size, /* stack size */
		(UINT) attr->priority, /* priority */
		0, /* preempt threshold */
		TX_NO_TIME_SLICE, /* time slice */
		TX_DONT_START /* auto start */
	);
	tx_thread_entry_exit_notify(new_thread, &entry_exit_notify);
	tx_thread_resume(new_thread);
	return (osThreadId_t) new_thread;
}

const char *osThreadGetName (osThreadId_t thread_id) {
	char* name = NULL;
	tx_thread_info_get(
		(TX_THREAD*)thread_id,
		&name, /* name */
		NULL, /* state */
		NULL, /* run_count */
		NULL, /* priority */
		NULL, /* preempt_threshold */
		NULL, /* time_slice */
		NULL, /* next_thread */
		NULL /* next_suspended_thread */
	);
	return (const char *) name;
}

osPriority_t osThreadGetPriority (osThreadId_t thread_id) {
	UINT priority = 0;
	tx_thread_info_get(
		(TX_THREAD*)thread_id,
		NULL, /* name */
		NULL, /* state */
		NULL, /* run_count */
		&priority, /* priority */
		NULL, /* preempt_threshold */
		NULL, /* time_slice */
		NULL, /* next_thread */
		NULL /* next_suspended_thread */
	);
	return priority;
}

osThreadState_t osThreadGetState (osThreadId_t thread_id) {
	UINT state = 0;
	tx_thread_info_get(
		(TX_THREAD*)thread_id,
		NULL, /* name */
		&state, /* state */
		NULL, /* run_count */
		NULL, /* priority */
		NULL, /* preempt_threshold */
		NULL, /* time_slice */
		NULL, /* next_thread */
		NULL /* next_suspended_thread */
	);
	return (osThreadState_t) state;
}

__NO_RETURN void osThreadExit (void) {
	TX_THREAD *thread_id = tx_thread_identify();
	tx_thread_terminate(thread_id);
	/* Ideally, this is never hit */
	while (true) ;
}
