/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "azure_iot_mqtt.h"

#include <ctype.h>
#include <string.h>

#include "tx_api.h"

#include "nx_api.h"
#include "nxd_mqtt_client.h"

#include "azure_iot_cert.h"
#include "azure_iot_mqtt/sas_token.h"

#define USERNAME                "%s/%s/?api-version=2020-05-31-preview&model-id=%s"
#define PUBLISH_TELEMETRY_TOPIC "devices/%s/messages/events/"

#define DEVICE_MESSAGE_BASE  "messages/devicebound/"
#define DEVICE_MESSAGE_TOPIC "devices/%s/messages/devicebound/#"

#define DEVICE_TWIN_PUBLISH_TOPIC          "$iothub/twin/PATCH/properties/reported/?$rid=%d"
#define DEVICE_TWIN_REQUEST_TOPIC          "$iothub/twin/GET/?$rid=%d"
#define DEVICE_TWIN_RES_BASE               "$iothub/twin/res/"
#define DEVICE_TWIN_RES_TOPIC              "$iothub/twin/res/#"
#define DEVICE_TWIN_DESIRED_PROP_RES_BASE  "$iothub/twin/PATCH/properties/desired/"
#define DEVICE_TWIN_DESIRED_PROP_RES_TOPIC "$iothub/twin/PATCH/properties/desired/#"

#define DIRECT_METHOD_RECEIVE  "$iothub/methods/POST/"
#define DIRECT_METHOD_TOPIC    "$iothub/methods/POST/#"
#define DIRECT_METHOD_RESPONSE "$iothub/methods/res/%d/?$rid=%s"

#define MQTT_CLIENT_PRIORITY 2
#define MQTT_TIMEOUT         (10 * TX_TIMER_TICKS_PER_SECOND)
#define MQTT_KEEP_ALIVE      240

#define MQTT_QOS_0 0 // QoS 0 - Deliver at most once
#define MQTT_QOS_1 1 // QoS 1 - Deliver at least once
#define MQTT_QOS_2 2 // QoS 2 - Deliver exactly once

extern const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers;

UINT azure_iot_mqtt_register_direct_method_callback(
    AZURE_IOT_MQTT* azure_iot_mqtt, func_ptr_direct_method mqtt_direct_method_callback)
{
    if (azure_iot_mqtt == NULL || azure_iot_mqtt->cb_ptr_mqtt_invoke_direct_method != NULL)
    {
        return NX_PTR_ERROR;
    }

    azure_iot_mqtt->cb_ptr_mqtt_invoke_direct_method = mqtt_direct_method_callback;
    return NX_SUCCESS;
}

UINT azure_iot_mqtt_register_c2d_message_callback(
    AZURE_IOT_MQTT* azure_iot_mqtt, func_ptr_c2d_message mqtt_c2d_message_callback)
{
    if (azure_iot_mqtt == NULL || azure_iot_mqtt->cb_ptr_mqtt_c2d_message != NULL)
    {
        return NX_PTR_ERROR;
    }

    azure_iot_mqtt->cb_ptr_mqtt_c2d_message = mqtt_c2d_message_callback;
    return NX_SUCCESS;
}

UINT azure_iot_mqtt_register_device_twin_desired_prop_callback(
    AZURE_IOT_MQTT* azure_iot_mqtt, func_ptr_device_twin_desired_prop mqtt_device_twin_desired_prop_callback)
{
    if (azure_iot_mqtt == NULL || azure_iot_mqtt->cb_ptr_mqtt_device_twin_desired_prop_callback != NULL)
    {
        return NX_PTR_ERROR;
    }

    azure_iot_mqtt->cb_ptr_mqtt_device_twin_desired_prop_callback = mqtt_device_twin_desired_prop_callback;
    return NX_SUCCESS;
}

UINT azure_iot_mqtt_register_device_twin_prop_callback(
    AZURE_IOT_MQTT* azure_iot_mqtt, func_ptr_device_twin_prop mqtt_device_twin_prop_callback)
{
    if (azure_iot_mqtt == NULL || azure_iot_mqtt->cb_ptr_mqtt_device_twin_prop_callback != NULL)
    {
        return NX_PTR_ERROR;
    }

    azure_iot_mqtt->cb_ptr_mqtt_device_twin_prop_callback = mqtt_device_twin_prop_callback;
    return NX_SUCCESS;
}

static UINT tls_setup(NXD_MQTT_CLIENT* client,
    NX_SECURE_TLS_SESSION* tls_session,
    NX_SECURE_X509_CERT* cert,
    NX_SECURE_X509_CERT* trusted_cert)
{
    UINT status;

    AZURE_IOT_MQTT* azure_iot_mqtt = (AZURE_IOT_MQTT*)client->nxd_mqtt_packet_receive_context;

    for (UINT index = 0; index < TLS_REMOTE_CERTIFICATE_COUNT; ++index)
    {
        status = nx_secure_tls_remote_certificate_allocate(tls_session,
            &azure_iot_mqtt->tls_remote_certificate[index],
            azure_iot_mqtt->tls_remote_cert_buffer[index],
            sizeof(azure_iot_mqtt->tls_remote_cert_buffer[index]));
        if (status != NX_SUCCESS)
        {
            printf("Unable to allocate memory for interemediate CA certificate (0x%02x)\r\n", status);
            return status;
        }
    }

    // Add a CA Certificate to our trusted store for verifying incoming server certificates
    status = nx_secure_x509_certificate_initialize(trusted_cert,
        (UCHAR*)azure_iot_root_ca,
        azure_iot_root_ca_len,
        NX_NULL,
        0,
        NX_NULL,
        0,
        NX_SECURE_X509_KEY_TYPE_NONE);
    if (status != NX_SUCCESS)
    {
        printf("Unable to initialize CA certificate (0x%02x)\r\n", status);
        return status;
    }

    status = nx_secure_tls_trusted_certificate_add(tls_session, trusted_cert);
    if (status != NX_SUCCESS)
    {
        printf("Unable to add CA certificate to trusted store (0x%02x)\r\n", status);
        return status;
    }

    status = nx_secure_tls_session_packet_buffer_set(
        tls_session, azure_iot_mqtt->tls_packet_buffer, sizeof(azure_iot_mqtt->tls_packet_buffer));
    if (status != NX_SUCCESS)
    {
        printf("Could not set TLS session packet buffer (0x%02x)\r\n", status);
        return status;
    }

    // Add a timestamp function for time checking and timestamps in the TLS handshake
    nx_secure_tls_session_time_function_set(tls_session, azure_iot_mqtt->unix_time_get);

    return NX_SUCCESS;
}

static UINT mqtt_publish(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* topic, CHAR* message)
{
    UINT status = nxd_mqtt_client_publish(&azure_iot_mqtt->nxd_mqtt_client,
        topic,
        strlen(topic),
        message,
        strlen(message),
        NX_FALSE,
        MQTT_QOS_1,
        NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        printf("Failed to publish %s (0x%02x)\r\n", message, status);
    }

    return status;
}

static UINT mqtt_publish_float(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* topic, CHAR* label, float value)
{
    CHAR mqtt_message[200] = {0};

    snprintf(mqtt_message, sizeof(mqtt_message), "{\"%s\":%3.2f}", label, value);
    printf("Sending message %s\r\n", mqtt_message);

    return mqtt_publish(azure_iot_mqtt, topic, mqtt_message);
}

static UINT mqtt_publish_bool(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* topic, CHAR* label, bool value)
{
    CHAR mqtt_message[200] = {0};

    snprintf(mqtt_message, sizeof(mqtt_message), "{\"%s\":%s}", label, (value ? "true" : false));
    printf("Sending message %s\r\n", mqtt_message);

    return mqtt_publish(azure_iot_mqtt, topic, mqtt_message);
}

static UINT mqtt_respond_direct_method(
    AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* topic, CHAR* request_id, MQTT_DIRECT_METHOD_RESPONSE* response)
{
    snprintf(topic, MQTT_TOPIC_NAME_LENGTH, DIRECT_METHOD_RESPONSE, response->status, request_id);
    return mqtt_publish(azure_iot_mqtt, topic, response->message);
}

static VOID process_direct_method(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* topic, CHAR* message)
{
    int direct_method_receive_size       = sizeof(DIRECT_METHOD_RECEIVE) - 1;
    MQTT_DIRECT_METHOD_RESPONSE response = {0, {0}};

    CHAR direct_method_name[64] = {0};
    CHAR request_id[16]         = {0};

    CHAR* location = topic + direct_method_receive_size;
    CHAR* find;

    find = strchr(location, '/');
    if (find == 0)
    {
        return;
    }

    strncpy(direct_method_name, location, find - location);

    location = find;

    find = strstr(location, "$rid=");
    if (find == 0)
    {
        return;
    }

    location = find + 5;

    strcpy(request_id, location);

    printf("Received direct method=%s, id=%s, message=%s\r\n", direct_method_name, request_id, message);

    if (azure_iot_mqtt->cb_ptr_mqtt_invoke_direct_method == NULL)
    {
        printf("No callback is registered for MQTT direct method invoke\r\n");
        return;
    }

    azure_iot_mqtt->cb_ptr_mqtt_invoke_direct_method(direct_method_name, message, &response);

    mqtt_respond_direct_method(azure_iot_mqtt, topic, request_id, &response);
}

static VOID process_c2d_message(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* topic)
{
    CHAR key[64]   = {0};
    CHAR value[64] = {0};

    CHAR* location;
    CHAR* find;

    // Get to parameters list
    find = strstr(topic, ".to");
    if (find == 0)
    {
        return;
    }

    // Extract the first property
    find = strstr(find, "&");
    if (find == 0)
    {
        return;
    }

    location = find + 1;

    find = strstr(find, "=");
    if (find == 0)
    {
        return;
    }

    strncpy(key, location, find - location);

    location = find + 1;

    strcpy(value, location);

    printf("Received property key=%s, value=%s\r\n", key, value);

    if (azure_iot_mqtt->cb_ptr_mqtt_c2d_message == NULL)
    {
        printf("No callback is registered for MQTT cloud to device message processing\r\n");
        return;
    }

    azure_iot_mqtt->cb_ptr_mqtt_c2d_message(key, value);
}

static VOID process_device_twin_response(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* topic, CHAR* message)
{
    CHAR device_twin_res_status[16] = {0};

    // Parse the device twin response status
    CHAR* location = topic + sizeof(DEVICE_TWIN_RES_BASE) - 1;
    CHAR* find;

    find = strchr(location, '/');
    if (find == 0)
    {
        return;
    }

    strncpy(device_twin_res_status, location, find - location);

    printf("Processed device twin update response with status=%s\r\n", device_twin_res_status);

    azure_iot_mqtt->cb_ptr_mqtt_device_twin_prop_callback(message);
}

static VOID process_device_twin_desired_prop_update(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* topic, CHAR* message)
{
    printf("Received device twin desired property\r\n");

    azure_iot_mqtt->cb_ptr_mqtt_device_twin_desired_prop_callback(message);
}

static VOID mqtt_disconnect_cb(NXD_MQTT_CLIENT* client_ptr)
{
    printf("ERROR: MQTT disconnected, reconnecting...\r\n");

    AZURE_IOT_MQTT* azure_iot_mqtt = (AZURE_IOT_MQTT*)client_ptr;

    // Try and reconnect forever
    while (azure_iot_mqtt_connect(azure_iot_mqtt) != NX_SUCCESS)
    {
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}

static VOID mqtt_notify_cb(NXD_MQTT_CLIENT* client_ptr, UINT number_of_messages)
{
    UINT status;
    UINT actual_topic_length;
    UINT actual_message_length;

    AZURE_IOT_MQTT* azure_iot_mqtt = (AZURE_IOT_MQTT*)client_ptr->nxd_mqtt_packet_receive_context;

    tx_mutex_get(&azure_iot_mqtt->mqtt_mutex, TX_WAIT_FOREVER);

    // Get the mqtt client message
    status = nxd_mqtt_client_message_get(client_ptr,
        (UCHAR*)azure_iot_mqtt->mqtt_topic_buffer,
        sizeof(azure_iot_mqtt->mqtt_topic_buffer),
        &actual_topic_length,
        (UCHAR*)azure_iot_mqtt->mqtt_message_buffer,
        sizeof(azure_iot_mqtt->mqtt_message_buffer),
        &actual_message_length);
    if (status == NXD_MQTT_SUCCESS)
    {
        // Append null string terminators
        azure_iot_mqtt->mqtt_topic_buffer[actual_topic_length]     = 0;
        azure_iot_mqtt->mqtt_message_buffer[actual_message_length] = 0;

        printf("[MQTT Received] topic = %s\r\n", azure_iot_mqtt->mqtt_topic_buffer);

        if (strstr((CHAR*)azure_iot_mqtt->mqtt_topic_buffer, DIRECT_METHOD_RECEIVE))
        {
            process_direct_method(
                azure_iot_mqtt, azure_iot_mqtt->mqtt_topic_buffer, azure_iot_mqtt->mqtt_message_buffer);
        }
        else if (strstr((CHAR*)azure_iot_mqtt->mqtt_topic_buffer, DEVICE_MESSAGE_BASE))
        {
            process_c2d_message(azure_iot_mqtt, azure_iot_mqtt->mqtt_topic_buffer);
        }
        else if (strstr((CHAR*)azure_iot_mqtt->mqtt_topic_buffer, DEVICE_TWIN_RES_BASE))
        {
            process_device_twin_response(
                azure_iot_mqtt, azure_iot_mqtt->mqtt_topic_buffer, azure_iot_mqtt->mqtt_message_buffer);
        }
        else if (strstr((CHAR*)azure_iot_mqtt->mqtt_topic_buffer, DEVICE_TWIN_DESIRED_PROP_RES_BASE))
        {
            process_device_twin_desired_prop_update(
                azure_iot_mqtt, azure_iot_mqtt->mqtt_topic_buffer, azure_iot_mqtt->mqtt_message_buffer);
        }
        else
        {
            printf("Unknown topic, no custom processing specified\r\n");
        }
    }

    tx_mutex_put(&azure_iot_mqtt->mqtt_mutex);
}

// Interact with Azure MQTT
UINT azure_iot_mqtt_publish_float_telemetry(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* label, float value)
{
    CHAR mqtt_publish_topic[100] = {0};
    UINT status;

    tx_mutex_get(&azure_iot_mqtt->mqtt_mutex, TX_WAIT_FOREVER);

    snprintf(mqtt_publish_topic,
        sizeof(mqtt_publish_topic),
        PUBLISH_TELEMETRY_TOPIC,
        azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_client_id);
    printf("Sending telemetry with float value\r\n");

    status = mqtt_publish_float(azure_iot_mqtt, mqtt_publish_topic, label, value);

    tx_mutex_put(&azure_iot_mqtt->mqtt_mutex);

    return status;
}

UINT azure_iot_mqtt_publish_float_property(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* label, float value)
{
    CHAR mqtt_publish_topic[100] = {0};
    UINT status;

    tx_mutex_get(&azure_iot_mqtt->mqtt_mutex, TX_WAIT_FOREVER);

    snprintf(mqtt_publish_topic,
        sizeof(mqtt_publish_topic),
        DEVICE_TWIN_PUBLISH_TOPIC,
        azure_iot_mqtt->reported_property_version++);
    printf("Sending device twin update with float value\r\n");

    status = mqtt_publish_float(azure_iot_mqtt, mqtt_publish_topic, label, value);

    tx_mutex_put(&azure_iot_mqtt->mqtt_mutex);

    return status;
}

UINT azure_iot_mqtt_publish_bool_property(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* label, bool value)
{
    CHAR mqtt_publish_topic[100] = {0};
    UINT status;

    tx_mutex_get(&azure_iot_mqtt->mqtt_mutex, TX_WAIT_FOREVER);

    snprintf(mqtt_publish_topic,
        sizeof(mqtt_publish_topic),
        DEVICE_TWIN_PUBLISH_TOPIC,
        azure_iot_mqtt->reported_property_version++);
    printf("Sending device twin update with bool value\r\n");

    status = mqtt_publish_bool(azure_iot_mqtt, mqtt_publish_topic, label, value);

    tx_mutex_put(&azure_iot_mqtt->mqtt_mutex);

    return status;
}

UINT azure_iot_mqtt_device_twin_request(AZURE_IOT_MQTT* azure_iot_mqtt)
{
    CHAR mqtt_publish_topic[100];
    UINT status;

    printf("Requesting device twin model\r\n");

    snprintf(mqtt_publish_topic, sizeof(mqtt_publish_topic), DEVICE_TWIN_REQUEST_TOPIC, 0);

    // Publish an empty message to request the device twin
    if ((status = mqtt_publish(azure_iot_mqtt, mqtt_publish_topic, "{}")))
    {
        printf("Failed to publish device twin request (0x%02x)\r\n", status);
        return status;
    }

    return NX_SUCCESS;
}

UINT azure_iot_mqtt_create(AZURE_IOT_MQTT* azure_iot_mqtt,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    func_ptr_unix_time_get unix_time_get,
    CHAR* iot_hub_hostname,
    CHAR* iot_device_id,
    CHAR* iot_sas_key,
    CHAR* iot_model_id)
{
    UINT status;

    printf("Initializing MQTT client\r\n");

    if (azure_iot_mqtt == NULL)
    {
        printf("ERROR: azure_iot_mqtt is NULL\r\n");
        return NX_PTR_ERROR;
    }

    if (iot_hub_hostname[0] == 0 || iot_device_id[0] == 0 || iot_sas_key[0] == 0)
    {
        printf("ERROR: IoT Hub connection configuration is empty\r\n");
        return NX_PTR_ERROR;
    }

    memset(azure_iot_mqtt, 0, sizeof(&azure_iot_mqtt));

    // Stash the connection information
    azure_iot_mqtt->nx_dns            = nx_dns;
    azure_iot_mqtt->unix_time_get     = unix_time_get;
    azure_iot_mqtt->mqtt_device_id    = iot_device_id;
    azure_iot_mqtt->mqtt_sas_key      = iot_sas_key;
    azure_iot_mqtt->mqtt_hub_hostname = iot_hub_hostname;
    azure_iot_mqtt->mqtt_model_id     = iot_model_id;

    status = nxd_mqtt_client_create(&azure_iot_mqtt->nxd_mqtt_client,
        "MQTT client",
        azure_iot_mqtt->mqtt_device_id,
        strlen(azure_iot_mqtt->mqtt_device_id),
        nx_ip,
        nx_pool,
        azure_iot_mqtt->mqtt_client_stack,
        AZURE_IOT_MQTT_CLIENT_STACK_SIZE,
        MQTT_CLIENT_PRIORITY,
        NX_NULL,
        0);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Failed to create MQTT Client (0x%02x)\r\n", status);
        return status;
    }

    status = nxd_mqtt_client_receive_notify_set(&azure_iot_mqtt->nxd_mqtt_client, mqtt_notify_cb);
    if (status)
    {
        printf("Error in setting receive notify (0x%02x)\r\n", status);
        nxd_mqtt_client_delete(&azure_iot_mqtt->nxd_mqtt_client);
        return status;
    }

    status = nxd_mqtt_client_disconnect_notify_set(&azure_iot_mqtt->nxd_mqtt_client, mqtt_disconnect_cb);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error in seting disconnect notification (0x%02x)\r\n", status);
        nxd_mqtt_client_delete(&azure_iot_mqtt->nxd_mqtt_client);
        return status;
    }

    status = tx_mutex_create(&azure_iot_mqtt->mqtt_mutex, "Azure MQTT", TX_NO_INHERIT);
    if (status != TX_SUCCESS)
    {
        nxd_mqtt_client_delete(&azure_iot_mqtt->nxd_mqtt_client);
        return status;
    }

    // Set the receive context (highjacking the packet_receive_context) for callbacks
    azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_packet_receive_context = azure_iot_mqtt;

    return NXD_MQTT_SUCCESS;
}

UINT azure_iot_mqtt_delete(AZURE_IOT_MQTT* azure_iot_mqtt)
{
    nxd_mqtt_client_disconnect(&azure_iot_mqtt->nxd_mqtt_client);
    nxd_mqtt_client_delete(&azure_iot_mqtt->nxd_mqtt_client);
    tx_mutex_delete(&azure_iot_mqtt->mqtt_mutex);

    return NXD_MQTT_SUCCESS;
}

UINT azure_iot_mqtt_connect(AZURE_IOT_MQTT* azure_iot_mqtt)
{
    UINT status;
    CHAR mqtt_subscribe_topic[100];
    NXD_ADDRESS server_ip;

    printf("\tHub hostname: %s\r\n", azure_iot_mqtt->mqtt_hub_hostname);
    printf("\tDevice id: %s\r\n", azure_iot_mqtt->mqtt_device_id);
    printf("\tModel id: %s\r\n", azure_iot_mqtt->mqtt_model_id);

    // Create the username & password
    snprintf(azure_iot_mqtt->mqtt_username,
        AZURE_IOT_MQTT_USERNAME_SIZE,
        USERNAME,
        azure_iot_mqtt->mqtt_hub_hostname,
        azure_iot_mqtt->mqtt_device_id,
        azure_iot_mqtt->mqtt_model_id);

    if (!create_sas_token(azure_iot_mqtt->mqtt_sas_key,
            strlen(azure_iot_mqtt->mqtt_sas_key),
            azure_iot_mqtt->mqtt_hub_hostname,
            azure_iot_mqtt->mqtt_device_id,
            azure_iot_mqtt->unix_time_get(),
            azure_iot_mqtt->mqtt_password,
            AZURE_IOT_MQTT_PASSWORD_SIZE))
    {
        printf("ERROR: Unable to generate SAS token\r\n");
        return NX_PTR_ERROR;
    }

    status = nx_secure_tls_session_create(&azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_tls_session,
        &nx_crypto_tls_ciphers,
        azure_iot_mqtt->tls_metadata_buffer,
        sizeof(azure_iot_mqtt->tls_metadata_buffer));
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Could not create TLS Session (0x%02x)\r\n", status);
        return status;
    }

    status = nxd_mqtt_client_login_set(&azure_iot_mqtt->nxd_mqtt_client,
        azure_iot_mqtt->mqtt_username,
        strlen(azure_iot_mqtt->mqtt_username),
        azure_iot_mqtt->mqtt_password,
        strlen(azure_iot_mqtt->mqtt_password));
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Could not create Login Set (0x%02x)\r\n", status);
        nx_secure_tls_session_delete(&azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_tls_session);
        return status;
    }

    // Resolve the MQTT server IP address
    status = nxd_dns_host_by_name_get(azure_iot_mqtt->nx_dns,
        (UCHAR*)azure_iot_mqtt->mqtt_hub_hostname,
        &server_ip,
        NX_IP_PERIODIC_RATE,
        NX_IP_VERSION_V4);
    if (status != NX_SUCCESS)
    {
        printf("Unable to resolve DNS for MQTT Server %s (0x%02x)\r\n", azure_iot_mqtt->mqtt_hub_hostname, status);
        nx_secure_tls_session_delete(&azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_tls_session);
        return status;
    }

    status = nxd_mqtt_client_secure_connect(&azure_iot_mqtt->nxd_mqtt_client,
        &server_ip,
        NXD_MQTT_TLS_PORT,
        tls_setup,
        MQTT_KEEP_ALIVE,
        NX_TRUE,
        MQTT_TIMEOUT);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Could not connect to MQTT server (0x%02x)\r\n", status);
        nx_secure_tls_session_delete(&azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_tls_session);
        return status;
    }

    snprintf(mqtt_subscribe_topic, sizeof(mqtt_subscribe_topic), DEVICE_MESSAGE_TOPIC, azure_iot_mqtt->mqtt_device_id);
    status = nxd_mqtt_client_subscribe(
        &azure_iot_mqtt->nxd_mqtt_client, mqtt_subscribe_topic, strlen(mqtt_subscribe_topic), MQTT_QOS_0);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error in subscribing to server (0x%02x)\r\n", status);
        nx_secure_tls_session_delete(&azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_tls_session);
        return status;
    }

    status = nxd_mqtt_client_subscribe(
        &azure_iot_mqtt->nxd_mqtt_client, DIRECT_METHOD_TOPIC, strlen(DIRECT_METHOD_TOPIC), MQTT_QOS_0);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error in direct method subscribing to server (0x%02x)\r\n", status);
        nx_secure_tls_session_delete(&azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_tls_session);
        return status;
    }

    status = nxd_mqtt_client_subscribe(
        &azure_iot_mqtt->nxd_mqtt_client, DEVICE_TWIN_RES_TOPIC, strlen(DEVICE_TWIN_RES_TOPIC), MQTT_QOS_0);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error in device twin response subscribing to server (0x%02x)\r\n", status);
        nx_secure_tls_session_delete(&azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_tls_session);
        return status;
    }

    status = nxd_mqtt_client_subscribe(&azure_iot_mqtt->nxd_mqtt_client,
        DEVICE_TWIN_DESIRED_PROP_RES_TOPIC,
        strlen(DEVICE_TWIN_DESIRED_PROP_RES_TOPIC),
        MQTT_QOS_0);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error in device twin desired properties response subscribing to server (0x%02x)\r\n", status);
        return status;
    }

    printf("SUCCESS: MQTT client initialized\r\n\r\n");

    return NXD_MQTT_SUCCESS;
}

UINT azure_iot_mqtt_disconnect(AZURE_IOT_MQTT* azure_iot_mqtt)
{
    UINT status = nxd_mqtt_client_disconnect(&azure_iot_mqtt->nxd_mqtt_client);

    return status;
}
