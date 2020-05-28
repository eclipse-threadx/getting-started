/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "azure_iothub.h"

#include <stdio.h>

#include "azure/azure_mqtt.h"

static AZURE_MQTT azure_mqtt;

static void mqtt_device_twin_desired_prop(CHAR *message)
{
    printf("Received device twin updated properties: %s\r\n", message);
}

UINT azure_iothub_run(CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key)
{
    UINT status;
    // float temperature;
    float temperature = 18.8;

    // Create Azure MQTT
    status = azure_mqtt_create(&azure_mqtt, iot_hub_hostname, iot_device_id, iot_sas_key);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error: Failed to create Azure MQTT (0x%02x)\r\n", status);
        return status;
    }

    // Register callbacks
    // azure_mqtt_register_direct_method_callback(&azure_mqtt, mqtt_direct_method);
    // azure_mqtt_register_c2d_message_callback(&azure_mqtt, mqtt_c2d_message);
    azure_mqtt_register_device_twin_desired_prop_callback(&azure_mqtt, mqtt_device_twin_desired_prop);

    // Connect the Azure MQTT client
    status = azure_mqtt_connect(&azure_mqtt);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error: Failed to create Azure MQTT (0x%02x)\r\n", status);
        return status;
    }

    printf("Starting MQTT loop\r\n");
    while (true)
    {
        // temperature = BSP_TSENSOR_ReadTemp();
        temperature += 0.2;
        
        // Send the compensated temperature as a telemetry event
        azure_mqtt_publish_float_telemetry(&azure_mqtt, "temperature", temperature);

        // Send the compensated temperature as a device twin update
        azure_mqtt_publish_float_property(&azure_mqtt, "temperature", temperature);

        // Sleep for 10 seconds
        tx_thread_sleep(10 * TX_TIMER_TICKS_PER_SECOND);
    }

    return NXD_MQTT_SUCCESS;
}
