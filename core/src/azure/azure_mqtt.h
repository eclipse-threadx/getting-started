/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _AZURE_MQTT_H
#define _AZURE_MQTT_H

#include <stdbool.h>

#include "tx_api.h"

#include "nx_api.h"
#include "nxd_dns.h"
#include "nxd_mqtt_client.h"

#define AZURE_MQTT_USERNAME_SIZE        128
#define AZURE_MQTT_PASSWORD_SIZE        256

#define MQTT_CLIENT_STACK_SIZE          4096

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

typedef struct AZURE_MQTT_STRUCT
{
    NXD_MQTT_CLIENT                   nxd_mqtt_client;
    NX_DNS                           *nx_dns;
    TX_MUTEX                          azure_mqtt_mutex;

    CHAR                             *azure_mqtt_device_id;
    CHAR                             *azure_mqtt_sas_key;
    CHAR                             *azure_mqtt_hub_hostname;

    CHAR                              azure_mqtt_username[AZURE_MQTT_USERNAME_SIZE];
    CHAR                              azure_mqtt_password[AZURE_MQTT_PASSWORD_SIZE];
    
    UCHAR                             mqtt_client_stack[MQTT_CLIENT_STACK_SIZE];

    UCHAR                             tls_metadata_buffer[TLS_METADATA_BUFFER_SIZE];
    NX_SECURE_X509_CERT               tls_remote_certificate[TLS_REMOTE_CERTIFICATE_COUNT];
    UCHAR                             tls_remote_cert_buffer[TLS_REMOTE_CERTIFICATE_COUNT][TLS_REMOTE_CERTIFICATE_BUFFER];
    UCHAR                             tls_packet_buffer[TLS_PACKET_BUFFER];
    
    func_ptr_direct_method            cb_ptr_mqtt_invoke_direct_method;
    func_ptr_c2d_message              cb_ptr_mqtt_c2d_message;
    func_ptr_device_twin_desired_prop cb_ptr_mqtt_device_twin_desired_prop_callback;

    func_ptr_unix_time_get            unix_time_get;
} AZURE_MQTT;

UINT azure_mqtt_register_direct_method_callback(AZURE_MQTT *azure_mqtt, func_ptr_direct_method mqtt_direct_method_callback);
UINT azure_mqtt_register_c2d_message_callback(AZURE_MQTT *azure_mqtt, func_ptr_c2d_message mqtt_c2d_message_callback);
UINT azure_mqtt_register_device_twin_desired_prop_callback(AZURE_MQTT *azure_mqtt, func_ptr_device_twin_desired_prop mqtt_device_twin_desired_prop_update_callback);

UINT azure_mqtt_publish_float_property(AZURE_MQTT *azure_mqtt, CHAR* label, float value);
UINT azure_mqtt_publish_bool_property(AZURE_MQTT *azure_mqtt, CHAR* label, bool value);
UINT azure_mqtt_publish_string_property(AZURE_MQTT *azure_mqtt, CHAR* label, CHAR *value);
UINT azure_mqtt_publish_float_telemetry(AZURE_MQTT *azure_mqtt, CHAR* label, float value);

UINT azure_mqtt_create(
    AZURE_MQTT *azure_mqtt, NX_IP *nx_ip, NX_PACKET_POOL *nx_pool, NX_DNS *nx_dns, 
    func_ptr_unix_time_get unix_time_get, 
    CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key);
UINT azure_mqtt_delete(AZURE_MQTT *azure_mqtt);

UINT azure_mqtt_connect(AZURE_MQTT *azure_mqtt);
UINT azure_mqtt_disconnect(AZURE_MQTT *azure_mqtt);

#endif // _AZURE_MQTT_H