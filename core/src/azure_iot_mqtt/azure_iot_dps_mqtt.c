/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

// https://docs.microsoft.com/en-us/azure/iot-dps/iot-dps-mqtt-support

#include <string.h>

#include "azure_iot_cert.h"
#include "azure_iot_dps_mqtt.h"

#include "azure_iot_mqtt/sas_token.h"

#include "json_utils.h"

#define USERNAME               "%s/registrations/%s/api-version=2019-03-31"
#define DPS_REGISTER_BASE      "$dps/registrations/res"
#define DPS_REGISTER_SUBSCRIBE "$dps/registrations/res/#"
#define DPS_REGISTER_TOPIC     "$dps/registrations/PUT/iotdps-register/?$rid=1"
#define DPS_STATUS_TOPIC       "$dps/registrations/GET/iotdps-get-operationstatus/?$rid=1&operationId=%s"

#define MQTT_PRIORITY   2
#define MQTT_TIMEOUT    (10 * TX_TIMER_TICKS_PER_SECOND)
#define MQTT_KEEP_ALIVE 240

#define EVENT_FLAGS_SUCCESS 1
//#define EVENT_FLAGS_FAILED  2

static VOID processRetry(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* topic, CHAR* message)
{
    UINT status;
    jsmn_parser parser;
    jsmntok_t tokens[12];
    INT token_count;

    CHAR mqtt_publish_topic[256];
    CHAR operation_id[100];
    int retry_interval;

    CHAR* find = strstr(topic, "retry-after=");
    if (find == 0)
    {
        printf("Error: Unknown retry-after\r\n");
        return;
    }

    // extract retry interval
    retry_interval = atoi(find + 12);

    jsmn_init(&parser);

    token_count = jsmn_parse(&parser,
        azure_iot_mqtt->mqtt_receive_message_buffer,
        strlen(azure_iot_mqtt->mqtt_receive_message_buffer),
        tokens,
        12);

    if (!findJsonString(azure_iot_mqtt->mqtt_receive_message_buffer, tokens, token_count, "operationId", operation_id))
    {
        printf("ERROR: Failed to parse DPS operationId\r\n");
    }

    snprintf(mqtt_publish_topic, sizeof(mqtt_publish_topic), DPS_STATUS_TOPIC, operation_id);

    tx_thread_sleep(retry_interval * TX_TIMER_TICKS_PER_SECOND);

    status = mqtt_publish(azure_iot_mqtt, mqtt_publish_topic, "");
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to poll for DPS status (0x%04x)\r\n", status);
    }
}

static VOID processSuccess(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* topic, CHAR* message)
{
    //  UINT status;
    jsmn_parser parser;
    jsmntok_t tokens[64];
    INT token_count;

    jsmn_init(&parser);

    token_count = jsmn_parse(&parser,
        azure_iot_mqtt->mqtt_receive_message_buffer,
        strlen(azure_iot_mqtt->mqtt_receive_message_buffer),
        tokens,
        64);

    //    findJsonString(azure_iot_mqtt->mqtt_receive_message_buffer, tokens,
    //    token_count, "deviceId", deviceId);
    if (!findJsonString(azure_iot_mqtt->mqtt_receive_message_buffer, tokens,
                       token_count, "assignedHub",
                       azure_iot_mqtt->mqtt_hub_hostname)) {
      printf("ERROR: DPS failed to parse hostname\r\n");
    }
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
//            tx_mutex_put(&azure_iot_mqtt->mqtt_mutex);
            //            tx_event_flags_set(&azure_iot_mqtt->mqtt_event_flags, EVENT_FLAGS_RECEIVED_FAILED, TX_OR);
            continue;
        }

        // Append null string terminators
        azure_iot_mqtt->mqtt_receive_topic_buffer[actual_topic_length]     = 0;
        azure_iot_mqtt->mqtt_receive_message_buffer[actual_message_length] = 0;

        printf("[MQTT Received] topic = %s\r\n", azure_iot_mqtt->mqtt_receive_topic_buffer);

        if (strstr((CHAR*)azure_iot_mqtt->mqtt_receive_topic_buffer, DPS_REGISTER_BASE) == 0)
        {
            printf("ERROR: Unknown DPS topic\r\n");
            //            tx_event_flags_set(&azure_iot_mqtt->mqtt_event_flags,
            //            EVENT_FLAGS_RECEIVED_FAILED, TX_OR);
            continue;
        }

        // Parse the response status
        CHAR* location = azure_iot_mqtt->mqtt_receive_topic_buffer + sizeof(DPS_REGISTER_BASE);
        INT msg_status = atoi(location);

        switch (msg_status)
        {
            case 202:
                processRetry(azure_iot_mqtt,
                    azure_iot_mqtt->mqtt_receive_topic_buffer,
                    azure_iot_mqtt->mqtt_receive_message_buffer);
                break;

            case 200:
              processSuccess(azure_iot_mqtt,
                             azure_iot_mqtt->mqtt_receive_topic_buffer,
                             azure_iot_mqtt->mqtt_receive_message_buffer);
              tx_event_flags_set(&azure_iot_mqtt->mqtt_event_flags,
                                 EVENT_FLAGS_SUCCESS, TX_OR);
              break;
              
            default:
                // TODO return error here
                return;
        }
    }

    // tx_event_flags_set(&azure_iot_mqtt->mqtt_event_flags, EVENT_FLAGS_RECEIVED_SUCCESS, TX_OR);
    return;
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

    azure_iot_mqtt->nx_dns            = nx_dns;
    azure_iot_mqtt->unix_time_get     = unix_time_get;
    azure_iot_mqtt->mqtt_dps_endpoint = endpoint;
    azure_iot_mqtt->mqtt_dps_id_scope = id_scope;
    azure_iot_mqtt->mqtt_device_id    = registration_id;

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
    NXD_ADDRESS server_ip;
    CHAR mqtt_publish_payload[100];

    printf("\tDPS endpoint: %s\r\n", azure_iot_mqtt->mqtt_dps_endpoint);
    printf("\tId scope: %s\r\n", azure_iot_mqtt->mqtt_dps_id_scope);
    printf("\tRegistration id: %s\r\n", azure_iot_mqtt->mqtt_device_id);

    // Create the nxd_mqtt_client_secure_connect & password
    snprintf(azure_iot_mqtt->mqtt_username,
        AZURE_IOT_MQTT_USERNAME_SIZE,
        USERNAME,
        azure_iot_mqtt->mqtt_dps_id_scope,
        azure_iot_mqtt->mqtt_device_id);

    if (!create_dps_sas_token(azure_iot_mqtt->mqtt_sas_key,
            strlen(azure_iot_mqtt->mqtt_sas_key),
            azure_iot_mqtt->mqtt_dps_id_scope,
            azure_iot_mqtt->mqtt_device_id,
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
    status = nxd_dns_host_by_name_get(azure_iot_mqtt->nx_dns,
        (UCHAR*)azure_iot_mqtt->mqtt_dps_endpoint,
        &server_ip,
        NX_IP_PERIODIC_RATE,
        NX_IP_VERSION_V4);
    if (status != NX_SUCCESS)
    {
        printf("Error: Unable to resolve DNS for DPS MQTT Server %s (0x%04x)\r\n",
            azure_iot_mqtt->mqtt_dps_endpoint,
            status);
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
    snprintf(mqtt_publish_payload,
        sizeof(mqtt_publish_payload),
        "{\"registrationId\":\"%s\"}",
        azure_iot_mqtt->mqtt_device_id);
    status = mqtt_publish(azure_iot_mqtt, DPS_REGISTER_TOPIC, mqtt_publish_payload);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to publish DPS registration (0x%04x)\r\n", status);
    }

    // Wait for an event
    ULONG events = 0;
    tx_event_flags_get(&azure_iot_mqtt->mqtt_event_flags, EVENT_FLAGS_SUCCESS,
                        TX_OR_CLEAR, &events, 10 * NX_IP_PERIODIC_RATE);

    if (events != EVENT_FLAGS_SUCCESS) {
        printf("ERROR: Failed to resolve device from DPS\r\n");
        return NX_FALSE;
    }

    printf("SUCCESS: MQTT DPS client initialized\r\n\r\n");

    return NXD_MQTT_SUCCESS;
}

UINT azure_iot_dps_device_info_get(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* iothub_hostname, CHAR* device_id)
{
  // mechanism to get the DPS response information

  //    iothub_hostname = azure_iot_mqtt->
  //    device_id = azure_iot_mqtt->
  strncpy(iothub_hostname, azure_iot_mqtt->mqtt_hub_hostname, 256);

  return NXD_MQTT_SUCCESS;
}
