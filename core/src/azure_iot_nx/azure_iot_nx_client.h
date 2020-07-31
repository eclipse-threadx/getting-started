/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_IOT_NX_CLIENT_H
#define _AZURE_IOT_NX_CLIENT_H

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_provisioning_client.h"

#include "jsmn.h"

#include "azure_iot_ciphersuites.h"

#define NX_AZURE_IOT_STACK_SIZE 2048
#define AZURE_IOT_NX_STACK_SIZE 4096

// typedef void(*func_ptr_direct_method)(CHAR *, CHAR *, MQTT_DIRECT_METHOD_RESPONSE *);
// typedef void(*func_ptr_c2d_message)(CHAR *, CHAR *);
// typedef void(*func_ptr_device_twin_desired_prop)(CHAR *);
typedef void (*threadx_entry)(ULONG parameter);

typedef struct AZURE_IOT_NX_CLIENT_STRUCT
{
    NX_SECURE_X509_CERT root_ca_cert;
    UCHAR nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE];
    ULONG nx_azure_iot_thread_stack[NX_AZURE_IOT_STACK_SIZE / sizeof(ULONG)];

    NX_AZURE_IOT nx_azure_iot;
    NX_AZURE_IOT_HUB_CLIENT iothub_client;

    TX_THREAD telemetry_thread;
    ULONG telemetry_thread_stack[AZURE_IOT_NX_STACK_SIZE / sizeof(ULONG)];
    TX_THREAD c2d_thread;
    ULONG c2d_thread_stack[AZURE_IOT_NX_STACK_SIZE / sizeof(ULONG)];
    TX_THREAD direct_method_thread;
    ULONG direct_method_thread_stack[AZURE_IOT_NX_STACK_SIZE / sizeof(ULONG)];
    TX_THREAD device_twin_thread;
    ULONG device_twin_thread_stack[AZURE_IOT_NX_STACK_SIZE / sizeof(ULONG)];

    threadx_entry telemetry_thread_entry;
    threadx_entry direct_method_thread_entry;
    threadx_entry device_twin_thread_entry;
    threadx_entry c2d_thread_entry;
} AZURE_IOT_NX_CLIENT;

UINT azure_iot_nx_client_create(AZURE_IOT_NX_CLIENT* azure_iot_nx_client,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    UINT (*unix_time_callback)(ULONG* unix_time),
    CHAR* iot_hub_hostname,
    CHAR* iot_device_id,
    CHAR* iot_sas_key,
    CHAR* iot_model_id);
UINT azure_iot_nx_client_delete(AZURE_IOT_NX_CLIENT* azure_iot_nx_client);

UINT azure_iot_nx_client_connect(AZURE_IOT_NX_CLIENT* azure_iot_nx_client);
UINT azure_iot_nx_client_disconnect(AZURE_IOT_NX_CLIENT* azure_iot_nx_client);

UINT azure_iot_nx_client_enable_telemetry(AZURE_IOT_NX_CLIENT* azure_iot_nx_client, threadx_entry telemetry_entry);
UINT azure_iot_nx_client_enable_device_twin(AZURE_IOT_NX_CLIENT* azure_iot_nx_client, threadx_entry device_twin_entry);
UINT azure_iot_nx_client_enable_direct_method(
    AZURE_IOT_NX_CLIENT* azure_iot_nx_client, threadx_entry direct_method_entry);
UINT azure_iot_nx_client_enable_c2d(AZURE_IOT_NX_CLIENT* azure_iot_nx_client, threadx_entry c2d_entry);

UINT azure_iot_nx_client_publish_float_telemetry(
    AZURE_IOT_NX_CLIENT* azure_iot_nx_client, CHAR* key, float value, NX_PACKET* packet_ptr);
UINT azure_iot_nx_client_publish_float_property(AZURE_IOT_NX_CLIENT* azure_iot_nx_client, CHAR* key, float value);
UINT azure_iot_nx_client_publish_bool_property(AZURE_IOT_NX_CLIENT* azure_iot_nx_client, CHAR* key, bool value);

VOID printf_packet(NX_PACKET* packet_ptr);

#endif