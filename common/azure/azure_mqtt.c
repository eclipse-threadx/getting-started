#include "azure_mqtt.h"

#include <ctype.h>
#include <string.h>

#include "nx_api.h"
#include "nxd_mqtt_client.h"

#include "azure/cert.h"
#include "azure/sas_token.h"

#include "azure_config.h"
#include "networking.h"
#include "sntp_client.h"

#define USERNAME "%s/%s/?api-version=2018-06-30"
#define PUBLISH_TELEMETRY_TOPIC "devices/%s/messages/events/"

#define DEVICE_MESSAGE_BASE "messages/devicebound/"
#define DEVICE_MESSAGE_RES "devices/%s/" DEVICE_MESSAGE_BASE
#define DEVICE_MESSAGE_TOPIC DEVICE_MESSAGE_RES "#"

#define DEVICE_TWIN_BASE "$iothub/twin/"
#define DEVICE_TWIN_PUBLISH_TOPIC DEVICE_TWIN_BASE "PATCH/properties/reported/?$rid=%d"
#define DEVICE_TWIN_RES_BASE DEVICE_TWIN_BASE "res/"
#define DEVICE_TWIN_RES_BASE_SIZE sizeof(DEVICE_TWIN_RES_BASE) - 1
#define DEVICE_TWIN_RES_TOPIC DEVICE_TWIN_RES_BASE "#"

#define DIRECT_METHOD_BASE "$iothub/methods/"
#define DIRECT_METHOD_RECEIVE DIRECT_METHOD_BASE "POST/"
#define DIRECT_METHOD_TOPIC DIRECT_METHOD_RECEIVE "#"
#define DIRECT_METHOD_RESPONSE DIRECT_METHOD_BASE "res/%d/?$rid=%s"
#define DIRECT_METHOD_RECEIVE_SIZE sizeof(DIRECT_METHOD_RECEIVE) - 1

#define MQTT_CLIENT_STACK_SIZE 4096
#define MQTT_CLIENT_PRIORITY 2

#define MQTT_THREAD_STACK_SIZE 2048
#define MQTT_THREAD_PRIORITY 5

#define TLS_METADATA_BUFFER_SIZE (16 * 1024)
#define TLS_REMOTE_CERTIFICATE_COUNT 4
#define TLS_REMOTE_CERTIFICATE_BUFFER 4096
#define TLS_PACKET_BUFFER (4 * 1024)

#define MQTT_TIMEOUT (30 * TX_TIMER_TICKS_PER_SECOND)
#define MQTT_KEEP_ALIVE 240

#define MQTT_TOPIC_NAME_LENGTH 200
#define MQTT_MESSAGE_NAME_LENGTH 200

#define MQTT_QOS_0 0 // QoS 0 - Deliver at most once
#define MQTT_QOS_1 1 // QoS 1 - Deliver at least once
#define MQTT_QOS_2 2 // QoS 2 - Deliver exactly once

static NXD_MQTT_CLIENT mqtt_client;
static TX_THREAD mqtt_thread;
static UCHAR mqtt_client_stack[MQTT_CLIENT_STACK_SIZE];
static UCHAR mqtt_thread_stack[MQTT_THREAD_STACK_SIZE];

static UCHAR tls_metadata_buffer[TLS_METADATA_BUFFER_SIZE];
static NX_SECURE_X509_CERT tls_remote_certificate[TLS_REMOTE_CERTIFICATE_COUNT];
static UCHAR tls_remote_cert_buffer[TLS_REMOTE_CERTIFICATE_COUNT][TLS_REMOTE_CERTIFICATE_BUFFER];
static UCHAR tls_packet_buffer[TLS_PACKET_BUFFER];

static CHAR mqtt_topic_buffer[MQTT_TOPIC_NAME_LENGTH];
static UINT mqtt_topic_length;
static CHAR mqtt_message_buffer[MQTT_MESSAGE_NAME_LENGTH];
static UINT mqtt_message_length;

extern const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers;

static UINT mqtt_init();
static UINT mqtt_open();
static UINT mqtt_publish(CHAR *topic, CHAR *message);
static UINT mqtt_publish_float(CHAR  *topic, CHAR *label, float value);
static UINT mqtt_publish_bool(CHAR  *topic, CHAR *label, bool value);
static UINT mqtt_publish_string(CHAR  *topic, CHAR *label, CHAR *value);
static UINT mqtt_respond_direct_method(CHAR *topic, CHAR *request_id, MQTT_DIRECT_METHOD_RESPONSE *response);
static VOID process_device_twin_response(CHAR *topic);
static VOID process_direct_method(CHAR *topic, CHAR *message);
static VOID process_c2d_message(CHAR *topic);

static func_ptr_main_thread cb_ptr_mqtt_main_thread = NULL;
static func_ptr_direct_method cb_ptr_mqtt_invoke_direct_method = NULL;
static func_ptr_c2d_message cb_ptr_mqtt_c2d_message = NULL;

// Initialize Azure MQTT
bool azure_mqtt_register_main_thread_callback(func_ptr_main_thread mqtt_main_thread_callback)
{
    bool status = false;
    
    if (cb_ptr_mqtt_main_thread == NULL)
    {
        cb_ptr_mqtt_main_thread = mqtt_main_thread_callback;
        status = true;
    }
    
    return status;
}

bool azure_mqtt_register_direct_method_invoke_callback(func_ptr_direct_method mqtt_direct_method_invoke_callback)
{
    bool status = false;
    
    if (cb_ptr_mqtt_invoke_direct_method == NULL)
    {
        cb_ptr_mqtt_invoke_direct_method = mqtt_direct_method_invoke_callback;
        status = true;
    }
    
    return status;
}

bool azure_mqtt_register_c2d_message_callback(func_ptr_c2d_message mqtt_c2d_message_callback)
{
    bool status = false;
    
    if (cb_ptr_mqtt_c2d_message == NULL)
    {
        cb_ptr_mqtt_c2d_message = mqtt_c2d_message_callback;
        status = true;
    }
    
    return status;
}

bool azure_mqtt_start()
{
    printf("Initializing MQTT client\r\n");

    UINT status;

    status = mqtt_init();
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("MQTT init failed\r\n");
        return false;
    }

    status = mqtt_open();
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("MQTT open failed\r\n");
        nxd_mqtt_client_delete(&mqtt_client);
        return false;
    }

    if (cb_ptr_mqtt_main_thread == NULL)
    {
        printf("No callback is registered for main MQTT thread\r\n");
        nxd_mqtt_client_delete(&mqtt_client);
        return false;
    }
    else
    {
        status = tx_thread_create(&mqtt_thread,
            "MQTT Thread",
            cb_ptr_mqtt_main_thread,
            (ULONG)NULL,
            &mqtt_thread_stack,
            MQTT_THREAD_STACK_SIZE,
            MQTT_THREAD_PRIORITY,
            MQTT_THREAD_PRIORITY,
            TX_NO_TIME_SLICE,
            TX_AUTO_START);
    
        if (status != TX_SUCCESS)
        {
            printf("Unable to create MQTT thread (0x%02x)\r\n", status);
            return false;
        }
    }

    printf("SUCCESS: MQTT client initialized\r\n\r\n");

    return true;
}

// Interact with Azure MQTT
UINT azure_mqtt_publish_float_telemetry(CHAR* label, float value)
{
    CHAR mqtt_publish_topic[100] = { 0 };

    snprintf(mqtt_publish_topic, sizeof(mqtt_publish_topic), PUBLISH_TELEMETRY_TOPIC, iot_device_id);
    printf("Sending telemetry\r\n");

    return mqtt_publish_float(mqtt_publish_topic, label, value);
}

UINT azure_mqtt_publish_float_twin(CHAR* label, float value)
{
    CHAR mqtt_publish_topic[100] = { 0 };

    snprintf(mqtt_publish_topic, sizeof(mqtt_publish_topic), DEVICE_TWIN_PUBLISH_TOPIC, 1);
    printf("Sending device twin update with float value\r\n");

    return mqtt_publish_float(mqtt_publish_topic, label, value);
}

UINT azure_mqtt_publish_bool_twin(CHAR* label, bool value)
{
    CHAR mqtt_publish_topic[100] = { 0 };

    snprintf(mqtt_publish_topic, sizeof(mqtt_publish_topic), DEVICE_TWIN_PUBLISH_TOPIC, 1);
    printf("Sending device twin update with bool value\r\n");

    return mqtt_publish_bool(mqtt_publish_topic, label, value);
}

UINT azure_mqtt_publish_string_twin(CHAR* label, CHAR *value)
{
    CHAR mqtt_publish_topic[100] = { 0 };

    snprintf(mqtt_publish_topic, sizeof(mqtt_publish_topic), DEVICE_TWIN_PUBLISH_TOPIC, 1);
    printf("Sending device twin update with string value\r\n");

    return mqtt_publish_string(mqtt_publish_topic, label, value);
}

// Azure MQTT private methods
static VOID mqtt_notify(NXD_MQTT_CLIENT *client_ptr, UINT number_of_messages)
{
    UINT status;

    // Get the mqtt client message
    status = nxd_mqtt_client_message_get(
        &mqtt_client,
        (UCHAR *)mqtt_topic_buffer,
        sizeof(mqtt_topic_buffer),
        &mqtt_topic_length,
        (UCHAR *)mqtt_message_buffer,
        sizeof(mqtt_message_buffer),
        &mqtt_message_length);
    if (status != NXD_MQTT_SUCCESS)
    {
        return;
    }

    // Append null string terminators
    mqtt_topic_buffer[mqtt_topic_length] = 0;
    mqtt_message_buffer[mqtt_message_length] = 0;

    // Convert to lowercase
    for (CHAR *p = mqtt_message_buffer; *p; ++p)
    {
        *p = tolower((INT)*p);
    }

    printf("[Received] topic = %s, message = %s\r\n", mqtt_topic_buffer, mqtt_message_buffer);

    if (strstr((CHAR *)mqtt_topic_buffer, DIRECT_METHOD_RECEIVE))
    {
        process_direct_method(mqtt_topic_buffer, mqtt_message_buffer);
    }
    else if (strstr((CHAR *)mqtt_topic_buffer, DEVICE_TWIN_RES_BASE))
    {
        process_device_twin_response(mqtt_topic_buffer);
    }
    else if (strstr((CHAR *)mqtt_topic_buffer, DEVICE_MESSAGE_BASE))
    {
        process_c2d_message(mqtt_topic_buffer);
    }
    else
    {
        printf("Unknown topic, no custom processing specified\r\n");
    }
}

static UINT tls_setup(NXD_MQTT_CLIENT *client, NX_SECURE_TLS_SESSION *tls_session, NX_SECURE_X509_CERT *cert, NX_SECURE_X509_CERT *trusted_cert)
{
    UINT status;

    for (UINT index = 0; index < TLS_REMOTE_CERTIFICATE_COUNT; ++index)
    {
        status = nx_secure_tls_remote_certificate_allocate(
            tls_session,
            &tls_remote_certificate[index],
            tls_remote_cert_buffer[index],
            sizeof(tls_remote_cert_buffer[index]));
        if (status != NX_SUCCESS)
        {
            printf("Unable to allocate memory for interemediate CA certificate, ret = 0x%x\r\n", status);
            return status;
        }
    }

    // Add a CA Certificate to our trusted store for verifying incoming server certificates
    status = nx_secure_x509_certificate_initialize(
        trusted_cert,
        (UCHAR *)azure_iot_root_ca, azure_iot_root_ca_len,
        NX_NULL, 0,
        NX_NULL, 0,
        NX_SECURE_X509_KEY_TYPE_NONE);
    if (status != NX_SUCCESS)
    {
        printf("Unable to initialize CA certificate, ret = 0x%x\r\n", status);
        return status;
    }

    status = nx_secure_tls_trusted_certificate_add(tls_session, trusted_cert);
    if (status != NX_SUCCESS)
    {
        printf("Unable to add CA certificate to trusted store, ret=0x%x\r\n", status);
        return status;
    }

    status = nx_secure_tls_session_packet_buffer_set(
        tls_session,
        tls_packet_buffer, sizeof(tls_packet_buffer));
    if (status != NX_SUCCESS)
    {
        printf("Could not set TLS session packet buffer (0x%02x)\r\n", status);
        return status;
    }

    // Add a timestamp function for time checking and timestamps in the TLS handshake
    nx_secure_tls_session_time_function_set(tls_session, sntp_get_time);

    return NX_SUCCESS;
}

static UINT mqtt_init()
{
    UINT status;

    status = nxd_mqtt_client_create(
        &mqtt_client, "MQTT client",
        (CHAR *)iot_device_id, strlen(iot_device_id),
        &ip_0, &main_pool,
        mqtt_client_stack, MQTT_CLIENT_STACK_SIZE, 
        MQTT_CLIENT_PRIORITY,
        NX_NULL, 0);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Failed to create MQTT Client\r\n");
        return status;
    }

    return NXD_MQTT_SUCCESS;
}

static UINT mqtt_open()
{
    CHAR mqtt_username[128];
    CHAR mqtt_password[256];
    CHAR mqtt_subscribe_topic[100];
    NXD_ADDRESS server_ip;
    UINT status;

    snprintf(mqtt_username, sizeof(mqtt_username), USERNAME, iot_hub_hostname, iot_device_id);
    create_sas_token(
        (CHAR *)iot_sas_key, strlen(iot_sas_key),
        (CHAR *)iot_hub_hostname, (CHAR *)iot_device_id, sntp_get_time(),
        mqtt_password, sizeof(mqtt_password));

    snprintf(mqtt_subscribe_topic, sizeof(mqtt_subscribe_topic), DEVICE_MESSAGE_TOPIC, iot_device_id);

    if (iot_hub_hostname[0] == 0)
    {
        printf("ERROR: iot_hub_hostname is blank\r\n");
        return 1;
    }
    
    status = nx_secure_tls_session_create(
        &mqtt_client.nxd_mqtt_tls_session,
        &nx_crypto_tls_ciphers,
        tls_metadata_buffer,
        sizeof(tls_metadata_buffer));
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Could not create TLS Session (0x%02x)\r\n", status);
        return status;
    }

    status = nxd_mqtt_client_login_set(
        &mqtt_client,
        mqtt_username, strlen(mqtt_username),
        mqtt_password, strlen(mqtt_password));
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Could not create Login Set (0x%02x)\r\n", status);
        nx_secure_tls_session_delete(&mqtt_client.nxd_mqtt_tls_session);
        return status;
    }

    // Resolve the MQTT server IP address
    status = nxd_dns_host_by_name_get(&dns_client, (UCHAR *)iot_hub_hostname, &server_ip, NX_IP_PERIODIC_RATE, NX_IP_VERSION_V4);
    if (status != NX_SUCCESS)
    {
        printf("Unable to resolve DNS for MQTT Server %s (0x%02x)\r\n", iot_hub_hostname, status);
        return status;
    }

    status = nxd_mqtt_client_secure_connect(
        &mqtt_client,
        &server_ip, NXD_MQTT_TLS_PORT,
        tls_setup, MQTT_KEEP_ALIVE, NX_TRUE, MQTT_TIMEOUT);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Could not connect to MQTT server (0x%02x)\r\n", status);
        return status;
    }

    status = nxd_mqtt_client_subscribe(
        &mqtt_client,
        mqtt_subscribe_topic,
        strlen(mqtt_subscribe_topic),
        MQTT_QOS_0);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error in subscribing to server (0x%02x)\r\n", status);
        return status;
    }

    status = nxd_mqtt_client_subscribe(
        &mqtt_client,
        DIRECT_METHOD_TOPIC,
        strlen(DIRECT_METHOD_TOPIC),
        MQTT_QOS_0);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error in direct method subscribing to server (0x%02x)\r\n", status);
        return status;
    }

    status = nxd_mqtt_client_subscribe(
        &mqtt_client,
        DEVICE_TWIN_RES_TOPIC,
        strlen(DEVICE_TWIN_RES_TOPIC),
        MQTT_QOS_0);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error in device twin response subscribing to server (0x%02x)\r\n", status);
        return status;
    }

    status = nxd_mqtt_client_receive_notify_set(&mqtt_client, mqtt_notify);
    if (status)
    {
        printf("Error in setting receive notify (0x%02x)\r\n", status);
        return status;
    }

    return NXD_MQTT_SUCCESS;
}

static UINT mqtt_publish_float(CHAR  *topic, CHAR *label, float value)
{
    CHAR mqtt_message[200] = { 0 };

    snprintf(mqtt_message, sizeof(mqtt_message), "{\"%s\": %3.2f}", label, value);
    printf("Sending message %s\r\n", mqtt_message);

    return mqtt_publish(topic, mqtt_message);
}

static UINT mqtt_publish_bool(CHAR  *topic, CHAR *label, bool value)
{
    CHAR mqtt_message[200] = { 0 };

    snprintf(mqtt_message, sizeof(mqtt_message), "{\"%s\": %d}", label, value);
    printf("Sending message %s\r\n", mqtt_message);

    return mqtt_publish(topic, mqtt_message);
}

static UINT mqtt_publish_string(CHAR  *topic, CHAR *label, CHAR *value)
{
    CHAR mqtt_message[200] = { 0 };

    snprintf(mqtt_message, sizeof(mqtt_message), "{\"%s\": %s}", label, value);
    printf("Sending message %s\r\n", mqtt_message);

    return mqtt_publish(topic, mqtt_message);
}

static UINT mqtt_publish(CHAR *topic, CHAR *message)
{
    UINT status = nxd_mqtt_client_publish(&mqtt_client,
                                          topic, strlen(topic),
                                          message, strlen(message),
                                          NX_FALSE,
                                          MQTT_QOS_1,
                                          NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        printf("Failed to publish %s (0x%02x)\r\n", message, status);
    }

    return status;
}

static UINT mqtt_respond_direct_method(CHAR *topic, CHAR *request_id, MQTT_DIRECT_METHOD_RESPONSE *response)
{
    snprintf(topic, MQTT_TOPIC_NAME_LENGTH, DIRECT_METHOD_RESPONSE, response->status, request_id);
    return mqtt_publish(topic, response->message);
}

static VOID process_device_twin_response(CHAR *topic)
{
    CHAR device_twin_res_status[16] = { 0 };
    CHAR request_id[16] = { 0 };
    
    // Parse the device twin response status
    CHAR *location = topic + DEVICE_TWIN_RES_BASE_SIZE;
    CHAR *find;

    find = strchr(location, '/');
    if (find == 0)
    {
        return;
    }
    
    strncpy(device_twin_res_status, location, find - location);

    // Parse the request id from the device twin response
    location = find;

    find = strstr(location, "$rid=");
    if (find == 0)
    {
        return;
    }

    location = find + 5;
    
    find = strchr(location, '&');
    if (find == 0)
    {
        return;
    }

    strncpy(request_id, location, find - location);

    printf("Processed device twin update response with status=%s, id=%s\r\n", device_twin_res_status, request_id);
}

static VOID process_direct_method(CHAR *topic, CHAR *message)
{
    MQTT_DIRECT_METHOD_RESPONSE response = { 0, { 0 } };
    
    CHAR direct_method_name[64] = { 0 };
    CHAR request_id[16] = { 0 };

    CHAR *location = topic + DIRECT_METHOD_RECEIVE_SIZE;
    CHAR *find;

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

    if (cb_ptr_mqtt_invoke_direct_method == NULL)
    {
        printf("No callback is registered for MQTT direct method invoke\r\n");
        return;
    }
    
    cb_ptr_mqtt_invoke_direct_method(direct_method_name, message, &response);

    mqtt_respond_direct_method(topic, request_id, &response);
}

static VOID process_c2d_message(CHAR *topic)
{
    CHAR key[64] = { 0 };
    CHAR value[64] = { 0 };

    CHAR *location;
    CHAR *find;
        
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
    
    if (cb_ptr_mqtt_c2d_message == NULL)
    {
        printf("No callback is registered for MQTT cloud to device message processing\r\n");
        return;
    }
    
    cb_ptr_mqtt_c2d_message(key, value);
}
