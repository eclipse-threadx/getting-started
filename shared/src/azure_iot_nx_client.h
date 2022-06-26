/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_IOT_NX_CLIENT_H
#define _AZURE_IOT_NX_CLIENT_H

#include "nx_api.h"

#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_json_reader.h"
#include "nx_azure_iot_json_writer.h"
#include "nx_azure_iot_provisioning_client.h"

#include "azure_iot_ciphersuites.h"

#define NX_AZURE_IOT_STACK_SIZE  (2 * 1024)
#define AZURE_IOT_STACK_SIZE     (3 * 1024)
#define AZURE_IOT_HOST_NAME_SIZE 128
#define AZURE_IOT_DEVICE_ID_SIZE 64

#define AZURE_IOT_AUTH_MODE_UNKNOWN 0
#define AZURE_IOT_AUTH_MODE_SAS     1
#define AZURE_IOT_AUTH_MODE_CERT    2

typedef struct AZURE_IOT_NX_CONTEXT_STRUCT AZURE_IOT_NX_CONTEXT;

typedef void (*func_ptr_command_received)(
    AZURE_IOT_NX_CONTEXT*, const UCHAR*, USHORT, const UCHAR*, USHORT, UCHAR*, USHORT, VOID*, USHORT);
typedef void (*func_ptr_writable_property_received)(
    AZURE_IOT_NX_CONTEXT*, const UCHAR*, UINT, UCHAR*, UINT, NX_AZURE_IOT_JSON_READER*, UINT);
typedef void (*func_ptr_property_received)(
    AZURE_IOT_NX_CONTEXT*, const UCHAR*, UINT, UCHAR*, UINT, NX_AZURE_IOT_JSON_READER*, UINT);
typedef void (*func_ptr_properties_complete)(AZURE_IOT_NX_CONTEXT*);
typedef void (*func_ptr_timer)(AZURE_IOT_NX_CONTEXT*);

typedef ULONG (*func_ptr_unix_time_get)(VOID);

struct AZURE_IOT_NX_CONTEXT_STRUCT
{
    NX_SECURE_X509_CERT root_ca_cert;
    NX_SECURE_X509_CERT root_ca_cert_2;
    NX_SECURE_X509_CERT root_ca_cert_3;

    NX_IP* azure_iot_nx_ip;

    ULONG nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE / sizeof(ULONG)];
    ULONG nx_azure_iot_thread_stack[NX_AZURE_IOT_STACK_SIZE / sizeof(ULONG)];
    ULONG azure_iot_thread_stack[AZURE_IOT_STACK_SIZE / sizeof(ULONG)];

    UINT azure_iot_auth_mode;

    // pnp model id
    CHAR* azure_iot_model_id;
    UINT azure_iot_model_id_len;

    // pnp components
    CHAR* azure_iot_components[NX_AZURE_IOT_HUB_CLIENT_MAX_COMPONENT_LIST];
    UINT azure_iot_component_count;

    // auth config
    CHAR* azure_iot_device_sas_key;
    UINT azure_iot_device_sas_key_len;
    NX_SECURE_X509_CERT device_certificate;

    // dps connection config
    CHAR* azure_iot_dps_id_scope;
    UINT azure_iot_dps_id_scope_len;
    CHAR* azure_iot_dps_registration_id;
    UINT azure_iot_dps_registration_id_len;

    // hub connection config
    CHAR azure_iot_hub_hostname[AZURE_IOT_HOST_NAME_SIZE];
    UINT azure_iot_hub_hostname_len;
    CHAR azure_iot_hub_device_id[AZURE_IOT_DEVICE_ID_SIZE];
    UINT azure_iot_hub_device_id_len;

    TX_THREAD azure_iot_thread;
    TX_EVENT_FLAGS_GROUP events;
    TX_TIMER periodic_timer;

    NX_AZURE_IOT nx_azure_iot;

    UINT azure_iot_connection_status;

    // union DPS and Hub as they are used consecutively and will save space
    union CLIENT_UNION {
        NX_AZURE_IOT_HUB_CLIENT iothub;
        NX_AZURE_IOT_PROVISIONING_CLIENT dps;
    } client;

#define iothub_client client.iothub
#define dps_client    client.dps

    func_ptr_command_received command_received_cb;
    func_ptr_writable_property_received writable_property_received_cb;
    func_ptr_property_received property_received_cb;
    func_ptr_properties_complete properties_complete_cb;
    func_ptr_timer timer_cb;
};

UINT azure_nx_client_periodic_interval_set(AZURE_IOT_NX_CONTEXT* nx_context, INT interval);

UINT azure_iot_nx_client_publish_telemetry(AZURE_IOT_NX_CONTEXT* nx_context,
    CHAR* component_name_ptr,
    UINT (*append_properties)(NX_AZURE_IOT_JSON_WRITER* json_writer_ptr));

UINT azure_iot_nx_client_publish_properties(AZURE_IOT_NX_CONTEXT* nx_context,
    CHAR* component_name_ptr,
    UINT (*append_properties)(NX_AZURE_IOT_JSON_WRITER* json_writer_ptr));
UINT azure_iot_nx_client_publish_bool_property(
    AZURE_IOT_NX_CONTEXT* nx_context, CHAR* component_name_ptr, CHAR* property_ptr, bool value);

UINT azure_nx_client_respond_int_writable_property(AZURE_IOT_NX_CONTEXT* nx_context,
    CHAR* component_name_ptr,
    CHAR* property_ptr,
    INT value,
    INT http_status,
    INT version);
UINT azure_iot_nx_client_publish_int_writable_property(
    AZURE_IOT_NX_CONTEXT* nx_context, CHAR* component_ptr, CHAR* property_ptr, UINT value);

UINT azure_iot_nx_client_register_command_callback(
    AZURE_IOT_NX_CONTEXT* nx_context, func_ptr_command_received callback);
UINT azure_iot_nx_client_register_writable_property_callback(
    AZURE_IOT_NX_CONTEXT* nx_context, func_ptr_writable_property_received callback);
UINT azure_iot_nx_client_register_property_callback(
    AZURE_IOT_NX_CONTEXT* nx_context, func_ptr_property_received callback);
UINT azure_iot_nx_client_register_properties_complete_callback(
    AZURE_IOT_NX_CONTEXT* nx_context, func_ptr_properties_complete callback);
UINT azure_iot_nx_client_register_timer_callback(
    AZURE_IOT_NX_CONTEXT* nx_context, func_ptr_timer callback, int32_t interval);

UINT azure_iot_nx_client_add_component(AZURE_IOT_NX_CONTEXT* nx_context, CHAR* component_name);

UINT azure_iot_nx_client_sas_set(AZURE_IOT_NX_CONTEXT* context, CHAR* device_sas_key);
UINT azure_iot_nx_client_cert_set(AZURE_IOT_NX_CONTEXT* context,
    UCHAR* device_x509_cert,
    UINT device_x509_cert_len,
    UCHAR* device_x509_key,
    UINT device_x509_key_len);

UINT azure_iot_nx_client_create(AZURE_IOT_NX_CONTEXT* context,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    UINT (*unix_time_callback)(ULONG* unix_time),
    CHAR* iot_model_id,
    UINT iot_model_id_len);

UINT azure_iot_nx_client_hub_run(
    AZURE_IOT_NX_CONTEXT* nx_context, CHAR* iot_hub_hostname, CHAR* iot_hub_device_id, UINT (*network_connect)());
UINT azure_iot_nx_client_dps_run(
    AZURE_IOT_NX_CONTEXT* nx_context, CHAR* dps_id_scope, CHAR* dps_registration_id, UINT (*network_connect)());

#endif
