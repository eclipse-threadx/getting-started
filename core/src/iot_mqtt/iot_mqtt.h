/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _IOT_MQTT_H
#define _IOT_MQTT_H

#include <stdbool.h>

#include "tx_api.h"

#include "nx_api.h"
#include "nxd_dns.h"
#include "nxd_mqtt_client.h"

#define IOT_MQTT_USERNAME_SIZE          128
#define IOT_MQTT_PASSWORD_SIZE          256

#define IOT_MQTT_CLIENT_STACK_SIZE      4096

#define TLS_METADATA_BUFFER_SIZE        (12 * 1024)
#define TLS_REMOTE_CERTIFICATE_COUNT    2
#define TLS_REMOTE_CERTIFICATE_BUFFER   4096
#define TLS_PACKET_BUFFER               4096

typedef struct MQTT_DIRECT_METHOD_RESPONSE_STRUCT
{
    int  status;
    CHAR message[16];
} MQTT_DIRECT_METHOD_RESPONSE;

typedef void(*func_ptr_direct_method)(CHAR *, CHAR *, MQTT_DIRECT_METHOD_RESPONSE *);
typedef void(*func_ptr_c2d_message)(CHAR *, CHAR *);
typedef void(*func_ptr_device_twin_desired_prop)(CHAR *);
typedef ULONG(*func_ptr_unix_time_get)(VOID);

typedef struct IOT_MQTT_STRUCT
{
    NXD_MQTT_CLIENT                   nxd_mqtt_client;
    NX_DNS                           *nx_dns;
    TX_MUTEX                          iot_mqtt_mutex;

    CHAR                             *iot_mqtt_device_id;
    CHAR                             *iot_mqtt_sas_key;
    CHAR                             *iot_mqtt_hub_hostname;
    CHAR                             *iot_mqtt_model_id;

    CHAR                              iot_mqtt_username[IOT_MQTT_USERNAME_SIZE];
    CHAR                              iot_mqtt_password[IOT_MQTT_PASSWORD_SIZE];
    
    UCHAR                             iot_mqtt_client_stack[IOT_MQTT_CLIENT_STACK_SIZE];

    UCHAR                             tls_metadata_buffer[TLS_METADATA_BUFFER_SIZE];
    NX_SECURE_X509_CERT               tls_remote_certificate[TLS_REMOTE_CERTIFICATE_COUNT];
    UCHAR                             tls_remote_cert_buffer[TLS_REMOTE_CERTIFICATE_COUNT][TLS_REMOTE_CERTIFICATE_BUFFER];
    UCHAR                             tls_packet_buffer[TLS_PACKET_BUFFER];
    
    func_ptr_direct_method            cb_ptr_mqtt_invoke_direct_method;
    func_ptr_c2d_message              cb_ptr_mqtt_c2d_message;
    func_ptr_device_twin_desired_prop cb_ptr_mqtt_device_twin_desired_prop_callback;

    func_ptr_unix_time_get            unix_time_get;
} IOT_MQTT;

UINT iot_mqtt_register_direct_method_callback(IOT_MQTT *iot_mqtt, func_ptr_direct_method mqtt_direct_method_callback);
UINT iot_mqtt_register_c2d_message_callback(IOT_MQTT *iot_mqtt, func_ptr_c2d_message mqtt_c2d_message_callback);
UINT iot_mqtt_register_device_twin_desired_prop_callback(IOT_MQTT *azure_mqtt, func_ptr_device_twin_desired_prop mqtt_device_twin_desired_prop_update_callback);

UINT iot_mqtt_publish_float_property(IOT_MQTT *iot_mqtt, CHAR* label, float value);
UINT iot_mqtt_publish_bool_property(IOT_MQTT *iot_mqtt, CHAR* label, bool value);
UINT iot_mqtt_publish_string_property(IOT_MQTT *iot_mqtt, CHAR* label, CHAR *value);
UINT iot_mqtt_publish_float_telemetry(IOT_MQTT *iot_mqtt, CHAR* label, float value);

UINT iot_mqtt_create(
    IOT_MQTT *iot_mqtt, NX_IP *nx_ip, NX_PACKET_POOL *nx_pool, NX_DNS *nx_dns, 
    func_ptr_unix_time_get unix_time_get, 
    CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key,
    CHAR *iot_model_id);
UINT iot_mqtt_delete(IOT_MQTT *iot_mqtt);

UINT iot_mqtt_connect(IOT_MQTT *iot_mqtt);
UINT iot_mqtt_disconnect(IOT_MQTT *iot_mqtt);

#endif // _IOT_MQTT_H