/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "azure_iothub.h"

#include <stdio.h>

#include "azure/azure_mqtt.h"
#include "networking.h"
#include "sntp_client.h"
#include "sensor.h"

#define IOT_MODEL_ID "dtmi:microsoft:gsg;1"

static AZURE_MQTT azure_mqtt;
static void mqtt_device_twin_desired_prop(CHAR *message)
{
    printf("Received device twin updated properties: %s\r\n", message);
}

UINT azure_iothub_run(CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key)
{
    UINT status;

    // Create Azure MQTT
    //    status = azure_mqtt_create(&azure_mqtt, iot_hub_hostname, iot_device_id, iot_sas_key);
    status = azure_mqtt_create(&azure_mqtt, 
        &nx_ip, &nx_pool[0], &nx_dns_client,
        sntp_time_get,
        iot_hub_hostname, iot_device_id, iot_sas_key,
        IOT_MODEL_ID);
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

    lps22hb_t lps22hb_data= {0};
    hts221_data_t hts221_data= {0};
    lsm6dsl_data_t lsm6dsl_data= {0};
    lis2mdl_data_t lis2mdl_data= {0};

    printf("Starting MQTT loop\r\n");
    while (true)
    {
        /* Read data from Sensors */
        lps22hb_data = lps22hb_data_read();
        hts221_data = hts221_data_read();
        lsm6dsl_data = lsm6dsl_data_read();
        lis2mdl_data = lis2mdl_data_read();

        // Send the compensated temperature as a telemetry event
        azure_mqtt_publish_float_telemetry(&azure_mqtt, "temperature", lps22hb_data.temperature_degC);
        // Send the compensated temperature as a device twin update
        azure_mqtt_publish_float_property(&azure_mqtt, "temperature", lps22hb_data.temperature_degC);
        
        // Send the compensated Pressure as a telemetry event
        azure_mqtt_publish_float_telemetry(&azure_mqtt, "Pressure", lps22hb_data.pressure_hPa);
        // Send the compensated Pressure as a device twin update
        azure_mqtt_publish_float_property(&azure_mqtt, "Pressure", lps22hb_data.pressure_hPa);
        
        // Send the compensated Humidity as a telemetry event
        azure_mqtt_publish_float_telemetry(&azure_mqtt, "Humidity Percentage", hts221_data.humidity_perc);
        // Send the compensated Humidity as a device twin update
        azure_mqtt_publish_float_property(&azure_mqtt, "Humidity Percentage", hts221_data.humidity_perc);
      
        // Send the compensated Acceleration as a telemetry event
        azure_mqtt_publish_float_telemetry(&azure_mqtt, "Acceleration mg[0]", lsm6dsl_data.acceleration_mg[0]);
        // Send the compensated Acceleration as a device twin update
        azure_mqtt_publish_float_property(&azure_mqtt, "Acceleration mg[0]", lsm6dsl_data.acceleration_mg[0]);

        // Send the compensated Magnetic as a telemetry event
        azure_mqtt_publish_float_telemetry(&azure_mqtt, "Magnetic mg[0]", lis2mdl_data.magnetic_mG[0]);
        // Send the compensated Magnetic as a device twin update
        azure_mqtt_publish_float_property(&azure_mqtt, "Magnetic mg[0]", lis2mdl_data.magnetic_mG[0]);

        // Sleep for 10 seconds
        tx_thread_sleep(10 * TX_TIMER_TICKS_PER_SECOND);
    }

    return NXD_MQTT_SUCCESS;
}