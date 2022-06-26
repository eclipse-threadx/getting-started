/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>

#include "nx_azure_iot_hub_client.h"

#include "azure_iot_nx_client.h"

#define INITIAL_EXPONENTIAL_BACKOFF_IN_SEC     (3)
#define MAX_EXPONENTIAL_BACKOFF_IN_SEC         (10 * 60)
#define MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT (60)

static UINT exponential_retry_count;

static VOID exponential_backoff_reset()
{
    exponential_retry_count = 0;
}

static VOID exponential_backoff_with_jitter()
{
    double jitter_percent = (MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT / 100.0) * (rand() / ((double)RAND_MAX));
    UINT base_delay       = MAX_EXPONENTIAL_BACKOFF_IN_SEC;
    uint64_t delay;
    UINT backoff_seconds;

    // If the retry is 0, then we don't need to delay the first time
    if (exponential_retry_count++ == 0)
    {
        return;
    }

    if (exponential_retry_count < (sizeof(UINT) * 8))
    {
        delay = (uint64_t)((1 << exponential_retry_count) * INITIAL_EXPONENTIAL_BACKOFF_IN_SEC);
        if (delay <= (UINT)(-1))
        {
            base_delay = (UINT)delay;
        }
    }

    if (base_delay > MAX_EXPONENTIAL_BACKOFF_IN_SEC)
    {
        base_delay = MAX_EXPONENTIAL_BACKOFF_IN_SEC;
    }
    else
    {
        exponential_retry_count++;
    }

    backoff_seconds = (UINT)(base_delay * (1 + jitter_percent));

    printf("\r\nIoT connection backoff for %d seconds\r\n", backoff_seconds);
    tx_thread_sleep(backoff_seconds * NX_IP_PERIODIC_RATE);
}

static void iothub_connect(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;

    // Connect to IoT hub
    printf("\r\nInitializing Azure IoT Hub client\r\n");
    printf("\tHub hostname: %.*s\r\n", nx_context->azure_iot_hub_hostname_len, nx_context->azure_iot_hub_hostname);
    printf("\tDevice id: %.*s\r\n", nx_context->azure_iot_hub_device_id_len, nx_context->azure_iot_hub_device_id);
    printf("\tModel id: %.*s\r\n", nx_context->azure_iot_model_id_len, nx_context->azure_iot_model_id);

    if ((status = nx_azure_iot_hub_client_connect(&nx_context->iothub_client, NX_FALSE, NX_WAIT_FOREVER)))
    {
        printf("ERROR: nx_azure_iot_hub_client_connect (0x%08x)\r\n", status);
    }

    // stash the connection status to be used by the monitor loop
    nx_context->azure_iot_connection_status = status;
}

VOID connection_status_set(AZURE_IOT_NX_CONTEXT* nx_context, UINT connection_status)
{
    nx_context->azure_iot_connection_status = connection_status;

    if (nx_context->azure_iot_connection_status == NX_SUCCESS)
    {
        printf("SUCCESS: Connected to IoT Hub\r\n\r\n");
    }
}

//---------------------------------------------------------------------------------
//
//   +-------------+              +-------------+              +-------------+
//   |             | SUCCESS      |             |  CONNECTED   |             |
//   |    INIT     +------------->|   CONNECT   +------------->|  CONNECTED  |
//   |             |              |             |              |             |
//   +-------------+              +---+---------+              +------+------+ 
//          ^ REINITIALIZE       FAIL |     ^ RECONNECT               | DISCONNECT
//          |                         |     |                         |
//          +-------------------------+     +-------------------------+
//
//---------------------------------------------------------------------------------
VOID connection_monitor(
    AZURE_IOT_NX_CONTEXT* nx_context, UINT (*iot_initialize)(AZURE_IOT_NX_CONTEXT* nx_context), UINT (*network_connect)())
{
    // Check parameters
    if ((nx_context == NX_NULL) || (iot_initialize == NX_NULL))
    {
        return;
    }

    // Check if connected
    if (nx_context->azure_iot_connection_status == NX_SUCCESS)
    {
        // Reset the exponential
        exponential_backoff_reset();
        return;
    }

    // Disconnect
    if (nx_context->azure_iot_connection_status != NX_AZURE_IOT_NOT_INITIALIZED)
    {
        nx_azure_iot_hub_client_disconnect(&nx_context->iothub_client);
    }

    // Recover
    while (true)
    {
        switch (nx_context->azure_iot_connection_status)
        {
            // Something bad has happened with client state, we need to re-initialize it
            case NX_DNS_QUERY_FAILED:
            case NXD_MQTT_COMMUNICATION_FAILURE:
            case NXD_MQTT_ERROR_BAD_USERNAME_PASSWORD:
            case NXD_MQTT_ERROR_NOT_AUTHORIZED:
            {
                // Deinitialize iot hub client
                nx_azure_iot_hub_client_deinitialize(&nx_context->iothub_client);
            }

            // Fallthrough
            case NX_AZURE_IOT_NOT_INITIALIZED:
            {
                // Set the state to not initialized
                nx_context->azure_iot_connection_status = NX_AZURE_IOT_NOT_INITIALIZED;

                // Connect the network
                if (network_connect() != NX_SUCCESS)
                {
                    // Failed, sleep and break out to try again next time
                    tx_thread_sleep(5 * TX_TIMER_TICKS_PER_SECOND);
                    break;
                }

                // Initialize IoT Hub
                exponential_backoff_with_jitter();
                if (iot_initialize(nx_context) == NX_SUCCESS)
                {
                    // Connect IoT Hub
                    iothub_connect(nx_context);
                }
            }
            break;

            case NX_AZURE_IOT_SAS_TOKEN_EXPIRED:
            {
                printf("SAS token has expired\r\n");
            }

            // Fallthrough
            default:
            {
                // Connect IoT Hub
                exponential_backoff_with_jitter();
                iothub_connect(nx_context);
            }
            break;
        }

        // Check status
        if (nx_context->azure_iot_connection_status == NX_SUCCESS)
        {
            // Success!
            return;
        }
    }
}
