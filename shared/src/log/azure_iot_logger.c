#include "azure_iot_logger.h"

#include "gsg_log.h"

// TODO would be nice to put this straight into the NETX PACKET
uint8_t telemetry_buffer[1200];

bool process_logs(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT telemetry_length;
    NX_PACKET* packet_ptr;
    NX_AZURE_IOT_JSON_WRITER json_writer;

    if (gsg_log_empty())
    {
        printf("LOGS: skipping upload\r\n");
        return true;
    }

    printf("LOGS: uploading %d\r\n", gsg_log_size());

    if (nx_azure_iot_hub_client_telemetry_message_create(&nx_context->iothub_client, &packet_ptr, NX_WAIT_FOREVER))
    {
        return false;
    }

    if (nx_azure_iot_json_writer_with_buffer_init(&json_writer, telemetry_buffer, sizeof(telemetry_buffer)))
    {
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return false;
    }

    if (nx_azure_iot_json_writer_append_begin_object(&json_writer))
    {
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return false;
    }

    if (nx_azure_iot_json_writer_append_property_name(&json_writer, (UCHAR*)"Logs", sizeof("Logs") - 1))
    {
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return false;
    }

    if (nx_azure_iot_json_writer_append_begin_array(&json_writer))
    {
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return false;
    }

    queue_buffer* buffer_ptr;
    while (gsg_log_pop(&buffer_ptr))
    {
        if (nx_azure_iot_json_writer_append_begin_object(&json_writer) ||

            nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                (UCHAR*)"componentName",
                sizeof("componentName") - 1,
                (UCHAR*)buffer_ptr->message,
                strlen(buffer_ptr->message)) ||

            nx_azure_iot_json_writer_append_property_with_int32_value(
                &json_writer, (UCHAR*)"time", sizeof("time") - 1, buffer_ptr->time) ||

            nx_azure_iot_json_writer_append_property_with_int32_value(
                &json_writer, (UCHAR*)"LogLevel", sizeof("LogLevel") - 1, buffer_ptr->log_level) ||

            nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                (UCHAR*)"message",
                sizeof("message") - 1,
                (UCHAR*)buffer_ptr->message,
                strlen(buffer_ptr->message)) ||

            nx_azure_iot_json_writer_append_end_object(&json_writer))
        {
            nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
            gsg_log_destroy(buffer_ptr);
            printf("ERROR failed to upload logs\r\n");
            return false;
        }

        gsg_log_destroy(buffer_ptr);
    }

    if (nx_azure_iot_json_writer_append_end_array(&json_writer))
    {
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return false;
    }

    if (nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return false;
    }

    telemetry_length = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
    printf("Uploading logs %.*s\r\n", telemetry_length, telemetry_buffer);

    if (nx_azure_iot_hub_client_telemetry_send(
            &nx_context->iothub_client, packet_ptr, telemetry_buffer, telemetry_length, NX_WAIT_FOREVER))
    {
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return false;
    }

    return true;
}
