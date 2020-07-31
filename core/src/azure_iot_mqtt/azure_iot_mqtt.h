/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_IOT_MQTT_H
#define _AZURE_IOT_MQTT_H

#include <stdbool.h>

#include "tx_api.h"

#include "nx_api.h"
#include "nxd_dns.h"
#include "nxd_mqtt_client.h"

#define AZURE_IOT_MQTT_USERNAME_SIZE 128
#define AZURE_IOT_MQTT_PASSWORD_SIZE 256

#define MQTT_TOPIC_NAME_LENGTH   300
#define MQTT_MESSAGE_NAME_LENGTH 1000

#define AZURE_IOT_MQTT_CLIENT_STACK_SIZE 4096

#define TLS_METADATA_BUFFER_SIZE      (12 * 1024)
#define TLS_REMOTE_CERTIFICATE_COUNT  2
#define TLS_REMOTE_CERTIFICATE_BUFFER 4096
#define TLS_PACKET_BUFFER             4096

typedef struct AZURE_IOT_MQTT_STRUCT AZURE_IOT_MQTT;

typedef void (*func_ptr_direct_method)(AZURE_IOT_MQTT*, CHAR*, CHAR*);
typedef void (*func_ptr_c2d_message)(AZURE_IOT_MQTT*, CHAR*, CHAR*);
typedef void (*func_ptr_device_twin_desired_prop)(AZURE_IOT_MQTT*, CHAR*);
typedef void (*func_ptr_device_twin_prop)(AZURE_IOT_MQTT*, CHAR*);
typedef ULONG (*func_ptr_unix_time_get)(VOID);

struct AZURE_IOT_MQTT_STRUCT
{
    NXD_MQTT_CLIENT nxd_mqtt_client;
    NX_DNS* nx_dns;

    CHAR* mqtt_device_id;
    CHAR* mqtt_sas_key;
    CHAR* mqtt_hub_hostname;
    CHAR* mqtt_model_id;

    UINT reported_property_version;
    UINT desired_property_version;
    UINT direct_command_request_id;

    CHAR mqtt_username[AZURE_IOT_MQTT_USERNAME_SIZE];
    CHAR mqtt_password[AZURE_IOT_MQTT_PASSWORD_SIZE];

    CHAR mqtt_receive_topic_buffer[MQTT_TOPIC_NAME_LENGTH];
    CHAR mqtt_receive_message_buffer[MQTT_MESSAGE_NAME_LENGTH];

    UCHAR mqtt_client_stack[AZURE_IOT_MQTT_CLIENT_STACK_SIZE];

    UCHAR tls_metadata_buffer[TLS_METADATA_BUFFER_SIZE];
    NX_SECURE_X509_CERT tls_remote_certificate[TLS_REMOTE_CERTIFICATE_COUNT];
    UCHAR tls_remote_cert_buffer[TLS_REMOTE_CERTIFICATE_COUNT][TLS_REMOTE_CERTIFICATE_BUFFER];
    UCHAR tls_packet_buffer[TLS_PACKET_BUFFER];

    func_ptr_direct_method cb_ptr_mqtt_invoke_direct_method;
    func_ptr_c2d_message cb_ptr_mqtt_c2d_message;
    func_ptr_device_twin_desired_prop cb_ptr_mqtt_device_twin_desired_prop_callback;
    func_ptr_device_twin_prop cb_ptr_mqtt_device_twin_prop_callback;

    func_ptr_unix_time_get unix_time_get;
};

UINT azure_iot_mqtt_register_direct_method_callback(
    AZURE_IOT_MQTT* azure_iot_mqtt, func_ptr_direct_method mqtt_direct_method_callback);
UINT azure_iot_mqtt_register_c2d_message_callback(
    AZURE_IOT_MQTT* azure_iot_mqtt, func_ptr_c2d_message mqtt_c2d_message_callback);
UINT azure_iot_mqtt_register_device_twin_desired_prop_callback(
    AZURE_IOT_MQTT* azure_iot_mqtt, func_ptr_device_twin_desired_prop mqtt_device_twin_desired_prop_update_callback);
UINT azure_iot_mqtt_register_device_twin_prop_callback(
    AZURE_IOT_MQTT* azure_iot_mqtt, func_ptr_device_twin_prop mqtt_device_twin_prop_callback);

UINT azure_iot_mqtt_publish_float_property(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* label, float value);
UINT azure_iot_mqtt_publish_bool_property(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* label, bool value);
UINT azure_iot_mqtt_publish_float_telemetry(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* label, float value);
UINT azure_iot_mqtt_publish_int_desired_property(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* label, int value);
UINT azure_iot_mqtt_respond_int_desired_property(
    AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* label, int value, int http_status);
UINT azure_iot_mqtt_respond_direct_method(AZURE_IOT_MQTT* azure_iot_mqtt, UINT response);
UINT azure_iot_mqtt_device_twin_request(AZURE_IOT_MQTT* azure_iot_mqtt);

UINT azure_iot_mqtt_create(AZURE_IOT_MQTT* azure_iot_mqtt,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    func_ptr_unix_time_get unix_time_get,
    CHAR* iot_hub_hostname,
    CHAR* iot_device_id,
    CHAR* iot_sas_key,
    CHAR* iot_model_id);
UINT azure_iot_mqtt_delete(AZURE_IOT_MQTT* azure_iot_mqtt);

UINT azure_iot_mqtt_connect(AZURE_IOT_MQTT* azure_iot_mqtt);
UINT azure_iot_mqtt_disconnect(AZURE_IOT_MQTT* azure_iot_mqtt);

#endif // _AZURE_IOT_MQTT_H