/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>

#include "nx_azure_iot_hub_client.h"

#include "azure_iot_nx_client.h"

#ifndef SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC
#define SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC (10 * 60)
#endif /* SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC */

#ifndef SAMPLE_INITIAL_EXPONENTIAL_BACKOFF_IN_SEC
#define SAMPLE_INITIAL_EXPONENTIAL_BACKOFF_IN_SEC (3)
#endif /* SAMPLE_INITIAL_EXPONENTIAL_BACKOFF_IN_SEC */

#ifndef SAMPLE_MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT
#define SAMPLE_MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT (60)
#endif /* SAMPLE_MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT */

static UINT exponential_retry_count;

static VOID exponential_backoff_reset()
{
    exponential_retry_count = 0;
}

static UINT exponential_backoff_with_jitter()
{
    double jitter_percent = (SAMPLE_MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT / 100.0) * (rand() / ((double)RAND_MAX));
    UINT base_delay       = SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC;
    uint64_t delay;

    if (exponential_retry_count < (sizeof(UINT) * 8))
    {
        delay = (uint64_t)((1 << exponential_retry_count) * SAMPLE_INITIAL_EXPONENTIAL_BACKOFF_IN_SEC);
        if (delay <= (UINT)(-1))
        {
            base_delay = (UINT)delay;
        }
    }

    if (base_delay > SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC)
    {
        base_delay = SAMPLE_MAX_EXPONENTIAL_BACKOFF_IN_SEC;
    }
    else
    {
        exponential_retry_count++;
    }

    return ((UINT)(base_delay * (1 + jitter_percent)) * NX_IP_PERIODIC_RATE);
}

static void iothub_connect(AZURE_IOT_NX_CONTEXT* nx_context, UINT (*network_connect)())
{
    // Wait for network
    //    ULONG gateway_address;
    /*    while (nx_ip_gateway_address_get(nx_context->azure_iot_nx_ip, &gateway_address))
        {
            tx_thread_sleep(NX_IP_PERIODIC_RATE);
        }*/
    network_connect();

    // Connect to IoT hub
    printf("Initializing Azure IoT Hub client\r\n");
    printf("\tHub hostname: %.*s\r\n", nx_context->azure_iot_hub_hostname_len, nx_context->azure_iot_hub_hostname);
    printf("\tDevice id: %.*s\r\n", nx_context->azure_iot_hub_device_id_len, nx_context->azure_iot_hub_device_id);
    printf("\tModel id: %.*s\r\n", nx_context->azure_iot_model_id_len, nx_context->azure_iot_model_id);

    nx_context->azure_iot_connection_status =
        nx_azure_iot_hub_client_connect(&nx_context->iothub_client, NX_FALSE, NX_WAIT_FOREVER);
}

VOID set_connection_status(AZURE_IOT_NX_CONTEXT* nx_context, UINT connection_status)
{
    nx_context->azure_iot_connection_status = connection_status;
}

//----------------------------------------------------------------------------------------------------
//
//   +-------------+           +-------------+           +-------------+           +-------------+
//   |             | SUCCESS   |             | SUCCESS   |             | CONNECTED |             |
//   |    INIT     +---------->|   NETWORK   +---------->|   CONNECT   +---------->|  CONNECTED  |
//   |             |           |             |           |             |           |             |
//   +-------------+           +-------------+           +------+------+           +------+------+
//          ^                         ^                         | FAIL                    | DISCONNECT
//          |                         |                         |                         |
//          |                         | RECONNECT               v                         |
//          |                  +------+--------------------------------+                  |
//          |     REINITIALIZE |                                       |                  |
//          +----------------- +             DISCONNECTED              |<-----------------+
//                             |                                       |
//                             +---------------------------------------+
//
//----------------------------------------------------------------------------------------------------
VOID connection_monitor(
    AZURE_IOT_NX_CONTEXT* nx_context, UINT (*iothub_init)(AZURE_IOT_NX_CONTEXT* nx_context), UINT (*network_connect)())
{
    // Check parameters
    if ((nx_context == NX_NULL) || (iothub_init == NX_NULL))
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

            case NXD_MQTT_ERROR_BAD_USERNAME_PASSWORD:
            case NXD_MQTT_ERROR_NOT_AUTHORIZED:
            {
                // Deinitialize iot hub client
                nx_azure_iot_hub_client_deinitialize(&nx_context->iothub_client);
            }

            // Fallthrough
            case NX_AZURE_IOT_NOT_INITIALIZED:
            {
                //                printf("Re-initializing iothub connection, after backoff\r\n");
                tx_thread_sleep(exponential_backoff_with_jitter());

                // Initialize iot hub
                if (iothub_init(nx_context))
                {
                    nx_context->azure_iot_connection_status = NX_AZURE_IOT_NOT_INITIALIZED;
                }
                else
                {
                    iothub_connect(nx_context, network_connect);
                }
            }
            break;

            case NX_AZURE_IOT_SAS_TOKEN_EXPIRED:
            {
                printf("SAS token expired\r\n");
            }

            // Fallthrough
            default:
            {
                printf("Reconnecting iothub, after backoff\r\n");

                tx_thread_sleep(exponential_backoff_with_jitter());

                iothub_connect(nx_context, network_connect);
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
