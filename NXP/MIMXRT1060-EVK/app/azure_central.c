#include "azure_central.h"

#include "nx_api.h"
#include "nxd_mqtt_client.h"

#include "network.h"

#define MQTT_THREAD_STACK_SIZE 2048
#define MQTT_THREAD_PRIORITY 4
#define MQTT_MEMORY_SIZE (10 * sizeof(MQTT_MESSAGE_BLOCK))

static UCHAR mqtt_thread_stack[MQTT_THREAD_STACK_SIZE];
static UCHAR mqtt_memory_block[MQTT_MEMORY_SIZE];
static NXD_MQTT_CLIENT mqtt_client;

bool azure_central_application()
{
	// define the thread for running Azure SDK on Azure RTOS

	UINT status;	
	
	// status = nxd_mqtt_client_create(&mqtt_client,
	// 	"client_name", 
	// 	"client_id",
	// 	strlen("client_id"), 
	// 	&ip_0,
	// 	&pool_0, 
	// 	mqtt_thread_stack,
	// 	MQTT_THREAD_STACK_SIZE,
	// 	MQTT_THREAD_PRIORITY, 
	// 	&mqtt_memory_block,
	// 	MQTT_MEMORY_SIZE);

	return false;
}
