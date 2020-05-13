#ifndef _AZURE_MQTT_H
#define _AZURE_MQTT_H

#include <stdbool.h>
#include <tx_port.h>

typedef struct MQTT_DIRECT_METHOD_RESPONSE_STRUCT
{
    int                           status;
    CHAR                          message[16];
} MQTT_DIRECT_METHOD_RESPONSE;

typedef void(*func_ptr_main_thread)(ULONG);
typedef void(*func_ptr_direct_method)(CHAR *, CHAR *, MQTT_DIRECT_METHOD_RESPONSE *);
typedef void(*func_ptr_c2d_message)(CHAR *, CHAR *);

bool azure_mqtt_register_main_thread_callback(func_ptr_main_thread mqtt_main_thread_callback);
bool azure_mqtt_register_direct_method_invoke_callback(func_ptr_direct_method mqtt_direct_method_invoke_callback);
bool azure_mqtt_register_c2d_message_callback(func_ptr_c2d_message mqtt_c2d_message_callback);

bool azure_mqtt_start(CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key);
UINT azure_mqtt_publish_float_property(CHAR* label, float value);
UINT azure_mqtt_publish_bool_property(CHAR* label, bool value);
UINT azure_mqtt_publish_string_property(CHAR* label, CHAR *value);
UINT azure_mqtt_publish_float_telemetry(CHAR* label, float value);

#endif // _AZURE_MQTT_H