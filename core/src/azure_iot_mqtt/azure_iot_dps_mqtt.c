/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

// https://docs.microsoft.com/en-us/azure/iot-dps/iot-dps-mqtt-support

#include "azure_iot_dps_mqtt.h"
#include "azure_iot_cert.h"

#include "azure_iot_mqtt/sas_token.h"

#include <string.h>

#define USERNAME               "%s/registrations/%s/api-version=2019-03-31"
#define DPS_REGISTER_BASE      "$dps/registrations/res"
#define DPS_REGISTER_SUBSCRIBE "$dps/registrations/res/#"
#define DPS_REGISTER_PUBLISH   "$dps/registrations/PUT/iotdps-register/?$rid=1"
#define DPS_STATUS             "$dps/registrations/GET/iotdps-get-operationstatus/?$rid=1&operationId=%s"

#define MQTT_PRIORITY   2
#define MQTT_TIMEOUT    (10 * TX_TIMER_TICKS_PER_SECOND)
#define MQTT_KEEP_ALIVE 240

#define MQTT_QOS_0 0 // QoS 0 - Deliver at most once
#define MQTT_QOS_1 1 // QoS 1 - Deliver at least once
#define MQTT_QOS_2 2 // QoS 2 - Deliver exactly once

#define DPS_EVENT_FLAGS_RETRY   0
#define DPS_EVENT_FLAGS_SUCCESS 1

static UINT tls_setup(NXD_MQTT_CLIENT* client,
    NX_SECURE_TLS_SESSION* tls_session,
    NX_SECURE_X509_CERT* cert,
    NX_SECURE_X509_CERT* trusted_cert)
{
    UINT status;

    AZURE_IOT_MQTT* azure_iot_mqtt = (AZURE_IOT_MQTT*)client->nxd_mqtt_packet_receive_context;

    // Create TLS session.
    status = _nx_secure_tls_session_create_ext(tls_session,
        _nx_azure_iot_tls_supported_crypto,
        _nx_azure_iot_tls_supported_crypto_size,
        _nx_azure_iot_tls_ciphersuite_map,
        _nx_azure_iot_tls_ciphersuite_map_size,
        azure_iot_mqtt->tls_metadata_buffer,
        sizeof(azure_iot_mqtt->tls_metadata_buffer));
    if (status != NX_SUCCESS)
    {
        printf("Failed to create TLS session status: %d\r\n", status);
        return status;
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

static VOID mqtt_notify_cb(NXD_MQTT_CLIENT* client_ptr, UINT number_of_messages)
{
    UINT actual_topic_length;
    UINT actual_message_length;
    UINT status;

    AZURE_IOT_MQTT* azure_iot_mqtt = (AZURE_IOT_MQTT*)client_ptr->nxd_mqtt_packet_receive_context;

    for (int count = 0; count < number_of_messages; ++count)
    {
        // Get the mqtt client message
        status = nxd_mqtt_client_message_get(client_ptr,
            (UCHAR*)azure_iot_mqtt->mqtt_receive_topic_buffer,
            AZURE_IOT_MQTT_TOPIC_NAME_LENGTH,
            &actual_topic_length,
            (UCHAR*)azure_iot_mqtt->mqtt_receive_message_buffer,
            AZURE_IOT_MQTT_MESSAGE_LENGTH,
            &actual_message_length);
        if (status != NXD_MQTT_SUCCESS)
        {
            printf("ERROR: nxd_mqtt_client_message_get failed (0x%02x)\r\n", status);
            continue;
        }

        // Append null string terminators
        azure_iot_mqtt->mqtt_receive_topic_buffer[actual_topic_length]     = 0;
        azure_iot_mqtt->mqtt_receive_message_buffer[actual_message_length] = 0;

        printf("[MQTT Received] topic = %s\r\n", azure_iot_mqtt->mqtt_receive_topic_buffer);

        if (strstr((CHAR*)azure_iot_mqtt->mqtt_receive_topic_buffer, DPS_REGISTER_BASE))
        {
            // Parse the device twin response status
            CHAR* location = azure_iot_mqtt->mqtt_receive_topic_buffer + sizeof(DPS_REGISTER_BASE);
            INT msg_status = atoi(location);

            if (msg_status == 200)
            {
                // Extract data
            }
            else if (msg_status == 202)
            {
                CHAR* find = strstr(location, "retry-after=");
                if (find == 0)
                {
                    printf("Error: Unknown retry-after\r\n");
                    return;
                }

                // flag a retry event after the retry interval
                azure_iot_mqtt->dps_retry_interval = atoi(find + 12);
                tx_event_flags_set(&azure_iot_mqtt->mqtt_event_flags, DPS_EVENT_FLAGS_RETRY, TX_OR);
            }
            else
            {
              printf("Unknown msg_status %d, msg=%s\r\n", msg_status,
                     azure_iot_mqtt->mqtt_receive_message_buffer);
            }
        }
        else
        {
            printf("Unknown topic, no custom processing specified\r\n");
        }
    }
}

UINT azure_iot_dps_create(AZURE_IOT_MQTT* azure_iot_mqtt,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    func_ptr_unix_time_get unix_time_get,
    CHAR* endpoint,
    CHAR* id_scope,
    CHAR* registration_id)
{
    UINT status;

    printf("Initializing MQTT DPS client\r\n");

    memset(azure_iot_mqtt, 0, sizeof(*azure_iot_mqtt));

    azure_iot_mqtt->nx_dns                      = nx_dns;
    azure_iot_mqtt->unix_time_get               = unix_time_get;
    azure_iot_mqtt->mqtt_dps_endpoint           = endpoint;
    azure_iot_mqtt->mqtt_dps_id_scope           = id_scope;
    azure_iot_mqtt->mqtt_device_id = registration_id;

    status = tx_event_flags_create(&azure_iot_mqtt->mqtt_event_flags, "DPS event flags");
    if (status != TX_SUCCESS)
    {
        printf("FAIL: Unable to create DPS event flags (0x%02x)\r\n", status);
        return false;
    }

    status = nxd_mqtt_client_create(&azure_iot_mqtt->nxd_mqtt_client,
        "MQTT DPS client",
        registration_id,
        strlen(registration_id),
        nx_ip,
        nx_pool,
        azure_iot_mqtt->mqtt_client_stack,
        AZURE_IOT_MQTT_CLIENT_STACK_SIZE,
        MQTT_PRIORITY,
        NX_NULL,
        0);
    if (status)
    {
        printf("Failed to create MQTT Client (0x%02x)\r\n", status);
        tx_event_flags_delete(&azure_iot_mqtt->mqtt_event_flags);
        return status;
    }

    status = nxd_mqtt_client_receive_notify_set(&azure_iot_mqtt->nxd_mqtt_client, mqtt_notify_cb);
    if (status)
    {
        printf("Error in setting receive notify (0x%02x)\r\n", status);
        tx_event_flags_delete(&azure_iot_mqtt->mqtt_event_flags);
        nxd_mqtt_client_delete(&azure_iot_mqtt->nxd_mqtt_client);
        return status;
    }

    // Set the receive context (highjacking the packet_receive_context) for callbacks
    azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_packet_receive_context = azure_iot_mqtt;

    return NX_SUCCESS;
}

UINT azure_iot_dps_delete(AZURE_IOT_MQTT* azure_iot_mqtt)
{
    if (azure_iot_mqtt == NX_NULL)
    {
        printf("Fail to delete DPS, null pointer\r\n");
        return NX_PTR_ERROR;
    }

    tx_event_flags_delete(&azure_iot_mqtt->mqtt_event_flags);
    nxd_mqtt_client_disconnect(&azure_iot_mqtt->nxd_mqtt_client);
    nxd_mqtt_client_delete(&azure_iot_mqtt->nxd_mqtt_client);

    return NX_SUCCESS;
}

UINT azure_iot_dps_symmetric_key_set(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* symmetric_key)
{
    if (azure_iot_mqtt == NX_NULL)
    {
        printf("Fail to set symmetric key, null pointer\r\n");
        return NX_PTR_ERROR;
    }

    azure_iot_mqtt->mqtt_sas_key = symmetric_key;

    return NX_SUCCESS;
}

UINT azure_iot_dps_register(AZURE_IOT_MQTT* azure_iot_mqtt, UINT wait)
{
    UINT status;
    ULONG events = 0;
    NXD_ADDRESS server_ip;
//    CHAR mqtt_publish_topic[100];
    CHAR message[100];

    printf("\tDPS endpoint: %s\r\n", azure_iot_mqtt->mqtt_dps_endpoint);
    printf("\tId scope: %s\r\n", azure_iot_mqtt->mqtt_dps_id_scope);
    printf("\tRegistration id: %s\r\n", azure_iot_mqtt->mqtt_device_id);

    // Create the nxd_mqtt_client_secure_connect & password
    snprintf(azure_iot_mqtt->mqtt_username, AZURE_IOT_MQTT_USERNAME_SIZE,
             USERNAME, azure_iot_mqtt->mqtt_dps_id_scope,
             azure_iot_mqtt->mqtt_device_id);

    if (!create_dps_sas_token(
            azure_iot_mqtt->mqtt_sas_key, strlen(azure_iot_mqtt->mqtt_sas_key),
            azure_iot_mqtt->mqtt_dps_id_scope, azure_iot_mqtt->mqtt_device_id,
            azure_iot_mqtt->unix_time_get(),
            azure_iot_mqtt->mqtt_password,
            AZURE_IOT_MQTT_PASSWORD_SIZE))
    {
        printf("ERROR: Unable to generate DPS SAS token\r\n");
        return NX_PTR_ERROR;
    }

    status = nxd_mqtt_client_login_set(&azure_iot_mqtt->nxd_mqtt_client,
        azure_iot_mqtt->mqtt_username,
        strlen(azure_iot_mqtt->mqtt_username),
        azure_iot_mqtt->mqtt_password,
        strlen(azure_iot_mqtt->mqtt_password));
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Could not set client login (0x%04x)\r\n", status);
        nx_secure_tls_session_delete(&azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_tls_session);
        return status;
    }

    // Resolve the MQTT server IP address
    status = nxd_dns_host_by_name_get(
        azure_iot_mqtt->nx_dns, (UCHAR *)azure_iot_mqtt->mqtt_dps_endpoint,
        &server_ip,
        NX_IP_PERIODIC_RATE,
        NX_IP_VERSION_V4);
    if (status != NX_SUCCESS)
    {
      printf("Error: Unable to resolve DNS for DPS MQTT Server %s (0x%04x)\r\n",
             azure_iot_mqtt->mqtt_dps_endpoint, status);
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
        printf("Error: Could not connect to DPS MQTT server (0x%04x)\r\n", status);
        nx_secure_tls_session_delete(&azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_tls_session);
        return status;
    }

    // Subscribe to the registration status topic
    status = nxd_mqtt_client_subscribe(
        &azure_iot_mqtt->nxd_mqtt_client, DPS_REGISTER_SUBSCRIBE, strlen(DPS_REGISTER_SUBSCRIBE), MQTT_QOS_0);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error: Error in DPS registration subscription (0x%04x)\r\n", status);
        nx_secure_tls_session_delete(&azure_iot_mqtt->nxd_mqtt_client.nxd_mqtt_tls_session);
        return status;
    }

    // Register the device
    snprintf(message, sizeof(message), "{\"registrationId\":\"%s\"}",
             azure_iot_mqtt->mqtt_device_id);
    status = nxd_mqtt_client_publish(&azure_iot_mqtt->nxd_mqtt_client,
        DPS_REGISTER_PUBLISH,
        strlen(DPS_REGISTER_PUBLISH),
        message,
        strlen(message),
        NX_FALSE,
        MQTT_QOS_1,
        NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        printf("DPS: Failed to publish registration %s (0x%04x)\r\n", message, status);
    }

    while (true)
    {
        tx_event_flags_get(
            &azure_iot_mqtt->mqtt_event_flags, DPS_EVENT_FLAGS_RETRY, TX_OR_CLEAR, &events, 10 * NX_IP_PERIODIC_RATE);

        printf("received DPS response\r\b");
        //        if ()

        // Wait for registration response

        // Poll for status
    }

    printf("SUCCESS: MQTT DPS client initialized\r\n\r\n");

    return NXD_MQTT_SUCCESS;
}

UINT azure_iot_dps_device_info_get(AZURE_IOT_MQTT* azure_iot_mqtt, UCHAR* iothub_hostname, UCHAR* device_id)
{
    // mechanism to get the DPS response information

//    iothub_hostname = azure_iot_mqtt->
//    device_id = azure_iot_mqtt->
    
    return NXD_MQTT_SUCCESS;
}
