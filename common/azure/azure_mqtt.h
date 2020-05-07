#ifndef _AZURE_MQTT_H
#define _AZURE_MQTT_H

#include <stdbool.h>
#include <tx_port.h>

typedef void(*func_ptr_t)(ULONG);
bool azure_mqtt_register_main_thread_callback(func_ptr_t mqtt_main_thread_callback);

bool azure_mqtt_start();
UINT mqtt_publish(CHAR *topic, CHAR *message);
UINT azure_mqtt_publish_float_twin(CHAR* label, float value);
UINT azure_mqtt_publish_float_telemetry(CHAR* label, float value);

#endif // _AZURE_MQTT_H