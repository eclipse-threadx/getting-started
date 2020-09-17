/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_IOT_NX_CLIENT_H
#define _AZURE_IOT_NX_CLIENT_H

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_provisioning_client.h"

#include "azure/core/az_json.h"

#include "azure_iot_ciphersuites.h"

#define NX_AZURE_IOT_STACK_SIZE  2048
#define AZURE_IOT_STACK_SIZE     2048
#define AZURE_IOT_HOST_NAME_SIZE 128
#define AZURE_IOT_DEVICE_ID_SIZE 64

typedef struct AZURE_IOT_NX_CONTEXT_STRUCT AZURE_IOT_NX_CONTEXT;

typedef void (*func_ptr_direct_method)(AZURE_IOT_NX_CONTEXT*, UCHAR*, USHORT, UCHAR*, USHORT, VOID*, USHORT);
typedef void (*func_ptr_device_twin_desired_prop)(UCHAR*, UINT, UCHAR*, UINT, az_json_reader, UINT, VOID*);
typedef void (*func_ptr_device_twin_prop)(UCHAR*, UINT, UCHAR*, UINT, az_json_reader, UINT, VOID*);
typedef ULONG (*func_ptr_unix_time_get)(VOID);

struct AZURE_IOT_NX_CONTEXT_STRUCT
{
    NX_SECURE_X509_CERT root_ca_cert;
    ULONG nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE / sizeof(ULONG)];
    ULONG nx_azure_iot_thread_stack[NX_AZURE_IOT_STACK_SIZE / sizeof(ULONG)];
    ULONG azure_iot_thread_stack[AZURE_IOT_STACK_SIZE / sizeof(ULONG)];

    TX_EVENT_FLAGS_GROUP events;

    CHAR azure_iot_hub_hostname[AZURE_IOT_HOST_NAME_SIZE];
    CHAR azure_iot_device_id[AZURE_IOT_DEVICE_ID_SIZE];

    TX_THREAD azure_iot_thread;

    NX_AZURE_IOT nx_azure_iot;

    union CLIENT_UNION {
        NX_AZURE_IOT_HUB_CLIENT iothub;
        NX_AZURE_IOT_PROVISIONING_CLIENT dps;
    } client;

#define iothub_client client.iothub
#define prov_client   client.dps

    func_ptr_direct_method direct_method_cb;
    func_ptr_device_twin_desired_prop device_twin_desired_prop_cb;
    func_ptr_device_twin_prop device_twin_get_cb;
};

UINT azure_iot_nx_client_register_direct_method(AZURE_IOT_NX_CONTEXT* context, func_ptr_direct_method callback);
UINT azure_iot_nx_client_register_device_twin_desired_prop(
    AZURE_IOT_NX_CONTEXT* context, func_ptr_device_twin_desired_prop callback);
UINT azure_iot_nx_client_register_device_twin_prop(AZURE_IOT_NX_CONTEXT* context, func_ptr_device_twin_prop callback);

UINT azure_iot_nx_client_create(AZURE_IOT_NX_CONTEXT* context,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    UINT (*unix_time_callback)(ULONG* unix_time),
    CHAR* iot_hub_hostname,
    CHAR* iot_device_id,
    CHAR* iot_sas_key,
    CHAR* iot_model_id);

UINT azure_iot_nx_client_dps_create(AZURE_IOT_NX_CONTEXT* context,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    UINT (*unix_time_callback)(ULONG* unix_time),
    CHAR* dps_endpoint,
    CHAR* dps_id_scope,
    CHAR* dps_registration_id,
    CHAR* device_sas_key,
    CHAR* device_model_id);

UINT azure_iot_nx_client_delete(AZURE_IOT_NX_CONTEXT* context);
UINT azure_iot_nx_client_connect(AZURE_IOT_NX_CONTEXT* context);

UINT azure_iot_nx_client_publish_float_telemetry(AZURE_IOT_NX_CONTEXT* context, CHAR* key, float value);
UINT azure_iot_nx_client_publish_float_property(AZURE_IOT_NX_CONTEXT* context, CHAR* key, float value);
UINT azure_iot_nx_client_publish_bool_property(AZURE_IOT_NX_CONTEXT* context, CHAR* key, bool value);

UINT azure_nx_client_respond_int_writeable_property(
    AZURE_IOT_NX_CONTEXT* context, CHAR* property, int value, int http_status, int version);

VOID printf_packet(NX_PACKET* packet_ptr, CHAR* prepend);

#endif