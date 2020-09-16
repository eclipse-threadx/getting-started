/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#include "nx_azure_iot_pnp_helpers.h"

#include <stdio.h>

#include "azure/core/az_json.h"
#include "nx_api.h"

/* Telemetry message property used to indicate the message's component. */
static const CHAR sample_pnp_telemetry_component_property[] = "$.sub";

/* Reported property response property keys */
static const az_span sample_pnp_component_type_property_name = AZ_SPAN_LITERAL_FROM_STR("__t");
static const az_span reported_component_type_value = AZ_SPAN_LITERAL_FROM_STR("c");
static const az_span reported_value_property_name = AZ_SPAN_LITERAL_FROM_STR("value");
static const az_span reported_status_property_name = AZ_SPAN_LITERAL_FROM_STR("ac");
static const az_span reported_version_property_name = AZ_SPAN_LITERAL_FROM_STR("av");
static const az_span reported_description_property_name = AZ_SPAN_LITERAL_FROM_STR("ad");

/* PnP command seperator */
static const az_span command_separator = AZ_SPAN_LITERAL_FROM_STR("*");

/* Device twin keys */
static const az_span sample_iot_hub_twin_desired_version = AZ_SPAN_LITERAL_FROM_STR("$version");
static const az_span sample_iot_hub_twin_desired = AZ_SPAN_LITERAL_FROM_STR("desired");

/* Move reader to the value of property name */
static UINT sample_json_child_token_move(az_json_reader *json_reader, az_span property_name)
{
    while (az_succeeded(az_json_reader_next_token(json_reader)))
    {
        if ((json_reader -> token.kind == AZ_JSON_TOKEN_PROPERTY_NAME) &&
            az_json_token_is_text_equal(&(json_reader -> token), property_name))
        {
           if (az_failed(az_json_reader_next_token(json_reader)))
           {
               printf("Failed to read next token\r\n");
               return(NX_NOT_SUCCESSFUL);
           }

           return(NX_AZURE_IOT_SUCCESS);
        }
        else if (json_reader -> token.kind == AZ_JSON_TOKEN_BEGIN_OBJECT)
        {
            if (az_failed(az_json_reader_skip_children(json_reader)))
            {
                printf("Failed to skip child of complex object\r\n");
                return(NX_NOT_SUCCESSFUL);
            }
        }
        else if (json_reader -> token.kind == AZ_JSON_TOKEN_END_OBJECT)
        {
            return(NX_AZURE_IOT_NOT_FOUND);
        }
    }

    return(NX_AZURE_IOT_NOT_FOUND);
}

/* Visit component property Object and call callback on each property of that component */
static UINT visit_component_properties(UCHAR *component_name_ptr, UINT component_name_len,
                                       az_json_reader *json_reader, UINT version, UCHAR *scratch_buf, UINT scratch_buf_len,
                                       VOID (*sample_desired_property_callback)(UCHAR *component_name_ptr,
                                             UINT component_name_len,
                                             UCHAR *property_name_ptr, UINT property_name_len,
                                             az_json_reader property_value_reader, UINT version,
                                             VOID *userContextCallback), VOID *context_ptr)
{
UINT len;

    while (az_succeeded(az_json_reader_next_token(json_reader)))
    {
        if (json_reader -> token.kind == AZ_JSON_TOKEN_PROPERTY_NAME)
        {
            if (az_failed(az_json_token_get_string(&(json_reader -> token), (CHAR *)scratch_buf, (INT)scratch_buf_len, (int32_t *)&len)))
            {
                printf("Failed to get string property value\r\n");
                return(NX_NOT_SUCCESSFUL);
            }

            if (az_failed(az_json_reader_next_token(json_reader)))
            {
                printf("Failed to get next token\r\n");
                return(NX_NOT_SUCCESSFUL);
            }

            if ((len == (UINT)az_span_size(sample_pnp_component_type_property_name))  &&
                (memcmp((VOID *)scratch_buf, (VOID *)az_span_ptr(sample_pnp_component_type_property_name), len) == 0))
            {
                continue;
            }

            if ((len == (UINT)az_span_size(sample_iot_hub_twin_desired_version)) &&
                (memcmp((VOID *)scratch_buf, (VOID *)az_span_ptr(sample_iot_hub_twin_desired_version), len) == 0))
            {
                continue;
            }

            sample_desired_property_callback(component_name_ptr, component_name_len,
                                             scratch_buf, len, *json_reader, version, context_ptr);

        }

        if (json_reader -> token.kind == AZ_JSON_TOKEN_BEGIN_OBJECT)
        {
            if (az_failed(az_json_reader_skip_children(json_reader)))
            {
                printf("Failed to skip children of object\r\n");
                return(NX_NOT_SUCCESSFUL);
            }
        }
        else if (json_reader -> token.kind == AZ_JSON_TOKEN_END_OBJECT)
        {
            break;
        }
    }

    return(NX_AZURE_IOT_SUCCESS);
}

/* Check if component is part of component list */
static UINT is_component_in_model(UCHAR *component_name_ptr, UINT component_name_len,
                                  CHAR **sample_components_ptr, UINT sample_components_num,
                                  UINT *out_index)
{
UINT index = 0;

    if (component_name_ptr == NX_NULL || component_name_len == 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    while (index < sample_components_num)
    {
        if ((component_name_len == strlen(sample_components_ptr[index])) &&
            (memcmp((VOID *)component_name_ptr, (VOID *)sample_components_ptr[index], component_name_len) == 0))
        {
            *out_index = index;
            return(NX_AZURE_IOT_SUCCESS);
        }

        index++;
    }

    return(NX_AZURE_IOT_NOT_FOUND);
}

/* Parse PnP command names*/
UINT nx_azure_iot_pnp_helper_command_name_parse(UCHAR *method_name_ptr, UINT method_name_length,
                                                UCHAR **component_name_pptr, UINT *component_name_length_ptr,
                                                UCHAR **pnp_command_name_pptr, UINT *pnp_command_name_length_ptr)
{
INT index;
az_span method_name = az_span_create(method_name_ptr, (INT)method_name_length);

    if ((index = az_span_find(method_name, command_separator)) != -1)
    {
        /* If a separator character is present in the device method name, then a command on a subcomponent of
           the model is being targeted (e.g. thermostat1*getMaxMinReport). */
        *component_name_pptr = method_name_ptr;
        *component_name_length_ptr = (UINT)index;
        *pnp_command_name_pptr = method_name_ptr + index + 1;
        *pnp_command_name_length_ptr = method_name_length - (UINT)index - 1;
    }
    else
    {
        /* The separator character is optional.  If it is not present, it indicates a command of the root
           component and not a subcomponent (e.g. "reboot"). */
        *component_name_pptr = NULL;
        *component_name_length_ptr = 0;
        *pnp_command_name_pptr = method_name_ptr;
        *pnp_command_name_length_ptr = method_name_length;
    }

    return(NX_AZURE_IOT_SUCCESS);
}

/* Parse twin data and call callback on each desired property */
UINT nx_azure_iot_pnp_helper_twin_data_parse(NX_PACKET *packet_ptr, UINT is_partial,
                                             CHAR **sample_components_ptr, UINT sample_components_num,
                                             UCHAR *scratch_buf, UINT scratch_buf_len,
                                             VOID (*sample_desired_property_callback)(UCHAR *component_name_ptr,
                                                   UINT component_name_len, UCHAR *property_name_ptr,
                                                   UINT property_name_len,
                                                   az_json_reader property_value_reader, UINT version,
                                                   VOID *userContextCallback),
                                             VOID *context_ptr)
{
az_json_reader json_reader;
az_json_reader copy_json_reader;
az_span payload;
UINT version;
UINT len;
UINT index;

    if (packet_ptr -> nx_packet_length >
        (ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr))
    {
        return(NX_NOT_SUCCESSFUL);
    }

    payload = az_span_create(packet_ptr -> nx_packet_prepend_ptr, (INT)(packet_ptr -> nx_packet_length));
    if (az_failed(az_json_reader_init(&json_reader, payload, NX_NULL)) ||
        az_failed(az_json_reader_next_token(&json_reader)))
    {
        printf("Failed to initialize json reader\r\n");
        return(NX_NOT_SUCCESSFUL);
    }

    if (!is_partial && sample_json_child_token_move(&json_reader, sample_iot_hub_twin_desired))
    {
        printf("Failed to get desired property\r\n");
        return(NX_NOT_SUCCESSFUL);
    }

    copy_json_reader = json_reader;
    if (sample_json_child_token_move(&copy_json_reader, sample_iot_hub_twin_desired_version) ||
        az_failed(az_json_token_get_int32(&(copy_json_reader.token), (int32_t *)&version)))
    {
        printf("Failed to get version\r\n");
        return(NX_NOT_SUCCESSFUL);
    }

    while (az_succeeded(az_json_reader_next_token(&json_reader)))
    {
        if (json_reader.token.kind == AZ_JSON_TOKEN_PROPERTY_NAME)
        {
            if (az_failed(az_json_token_get_string(&(json_reader.token), (CHAR *)scratch_buf,
                                                   (INT)scratch_buf_len, (int32_t *)&len)))
            {
                printf("Failed to string value for property name\r\n");
                return(NX_NOT_SUCCESSFUL);
            }

            if (az_failed(az_json_reader_next_token(&json_reader)))
            {
                printf("Failed to next token\r\n");
                return(NX_NOT_SUCCESSFUL);
            }

            if ((len == (UINT)az_span_size(sample_iot_hub_twin_desired_version)) &&
                (memcmp((VOID *)az_span_ptr(sample_iot_hub_twin_desired_version), (VOID *)scratch_buf,
                        (UINT)az_span_size(sample_iot_hub_twin_desired_version)) == 0))
            {
                continue;
            }

            if (json_reader.token.kind == AZ_JSON_TOKEN_BEGIN_OBJECT &&
                sample_components_ptr != NX_NULL &&
                (is_component_in_model(scratch_buf, len,
                                       sample_components_ptr, sample_components_num,
                                       &index) == NX_AZURE_IOT_SUCCESS))
            {
                if (visit_component_properties((UCHAR *)sample_components_ptr[index],
                                               strlen(sample_components_ptr[index]),
                                               &json_reader, version, scratch_buf, scratch_buf_len,
                                               sample_desired_property_callback, context_ptr))
                {
                    printf("Failed to visit component properties\r\n");
                    return(NX_NOT_SUCCESSFUL);
                }
            }
            else
            {
                sample_desired_property_callback(NX_NULL, 0, scratch_buf, len, json_reader, version, context_ptr);

                if (json_reader.token.kind == AZ_JSON_TOKEN_BEGIN_OBJECT)
                {
                    if (az_failed(az_json_reader_skip_children(&json_reader)))
                    {
                        printf("Failed to skip children of object\r\n");
                        return(NX_NOT_SUCCESSFUL);
                    }
                }
            }
        }
        else if (json_reader.token.kind == AZ_JSON_TOKEN_BEGIN_OBJECT)
        {
            if (az_failed(az_json_reader_skip_children(&json_reader)))
            {
                printf("Failed to skip children of object\r\n");
                return(NX_NOT_SUCCESSFUL);
            }
        }
        else if (json_reader.token.kind == AZ_JSON_TOKEN_END_OBJECT)
        {
            break;
        }
    }

    return(NX_AZURE_IOT_SUCCESS);
}

/* Create PnP telemetry message */
UINT nx_azure_iot_pnp_helper_telemetry_message_create(NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                                      UCHAR *component_name, UINT component_name_len,
                                                      NX_PACKET **packet_pptr, UINT wait_option)
{
UINT status;

    /* Create a telemetry message packet. */
    if ((status = nx_azure_iot_hub_client_telemetry_message_create(iothub_client_ptr, packet_pptr, wait_option)))
    {
        printf("Telemetry message create failed!: error code = 0x%08x\r\n", status);
    }
    /* If the component will be used, then specify this as a property of the message. */
    else if ((component_name != NULL) &&
             (status = nx_azure_iot_hub_client_telemetry_property_add(*packet_pptr,
                                                                      (UCHAR *)sample_pnp_telemetry_component_property,
                                                                      (USHORT)sizeof(sample_pnp_telemetry_component_property) - 1,
                                                                      component_name, (USHORT)component_name_len,
                                                                      NX_WAIT_FOREVER)) != NX_AZURE_IOT_SUCCESS)
    {
        printf("nx_azure_iot_hub_client_telemetry_property_add=%s failed, error=%d",
                sample_pnp_telemetry_component_property, status);
        nx_azure_iot_hub_client_telemetry_message_delete(*packet_pptr);
    }
    else
    {
        status = NX_AZURE_IOT_SUCCESS;
    }

    return(status);
}

/* Build PnP reported property into user provided buffer */
UINT nx_azure_iot_pnp_helper_build_reported_property(UCHAR *component_name_ptr, UINT component_name_len,
                                                     UINT (*append_reported_property)(az_json_writer *json_builder,
                                                                                      VOID *context),
                                                     VOID *context, UCHAR *buffer_ptr, UINT buffer_len,
                                                     UINT *data_copied_length_ptr )
{
UINT status;
az_span buff_span = az_span_create(buffer_ptr, (INT)buffer_len);
az_json_writer json_builder;
az_span component_name = az_span_create(component_name_ptr, (INT)component_name_len);

    if (az_succeeded(az_json_writer_init(&json_builder, buff_span, NULL)) &&
        az_succeeded(az_json_writer_append_begin_object(&json_builder)) &&
        (component_name_ptr == NX_NULL ||
         (az_succeeded(az_json_writer_append_property_name(&json_builder, component_name)) &&
          az_succeeded(az_json_writer_append_begin_object(&json_builder)) &&
          az_succeeded(az_json_writer_append_property_name(&json_builder, sample_pnp_component_type_property_name)) &&
          az_succeeded(az_json_writer_append_string(&json_builder, reported_component_type_value)))) &&
        (append_reported_property(&json_builder, context) == NX_AZURE_IOT_SUCCESS) &&
        (component_name_ptr == NX_NULL || az_succeeded(az_json_writer_append_end_object(&json_builder))) &&
        az_succeeded(az_json_writer_append_end_object(&json_builder)))
    {
        *data_copied_length_ptr  = (UINT)az_span_size(az_json_writer_get_bytes_used_in_destination(&json_builder));
        status = NX_AZURE_IOT_SUCCESS;
    }
    else
    {
        printf("Failed to build reported property\r\n");
        status = NX_NOT_SUCCESSFUL;
    }

    return(status);
}

/* Build reported property with status */
UINT nx_azure_iot_pnp_helper_build_reported_property_with_status(UCHAR *component_name_ptr, UINT component_name_len,
                                                                 UCHAR *property_name_ptr, UINT property_name_len,
                                                                 UINT (*append_value)(az_json_writer *builder,
                                                                                      VOID *context),
                                                                 VOID *context,
                                                                 INT result, UCHAR *description_ptr,
                                                                 UINT description_len,
                                                                 UINT ack_version, UCHAR *buffer_ptr,
                                                                 UINT buffer_len,
                                                                 UINT *byte_copied)
{
az_span buff_span = az_span_create(buffer_ptr, (INT)buffer_len);
az_json_writer json_builder;
az_span component_name = az_span_create(component_name_ptr, (INT)component_name_len);
az_span reported_property_name = az_span_create(property_name_ptr, (INT)property_name_len);
az_span description = az_span_create(description_ptr, (INT)description_len);

    if (az_failed(az_json_writer_init(&json_builder, buff_span, NULL)) ||
        az_failed(az_json_writer_append_begin_object(&json_builder)))
    {
        printf("Failed initializing json writer \r\n");
        return(NX_NOT_SUCCESSFUL);
    }

    if (component_name_ptr != NX_NULL &&
        !(az_succeeded(az_json_writer_append_property_name(&json_builder, component_name)) &&
          az_succeeded(az_json_writer_append_begin_object(&json_builder)) &&
          az_succeeded(az_json_writer_append_property_name(&json_builder, sample_pnp_component_type_property_name)) &&
          az_succeeded(az_json_writer_append_string(&json_builder, reported_component_type_value))))
    {
        printf("Failed build reported property with status message \r\n");
        return(NX_NOT_SUCCESSFUL);
    }

    if (!(az_succeeded(az_json_writer_append_property_name(&json_builder, reported_property_name)) &&
          az_succeeded(az_json_writer_append_begin_object(&json_builder)) &&
          az_succeeded(az_json_writer_append_property_name(&json_builder, reported_value_property_name)) &&
          (append_value(&json_builder, context) == NX_AZURE_IOT_SUCCESS) &&
          az_succeeded(az_json_writer_append_property_name(&json_builder, reported_status_property_name)) &&
          az_succeeded(az_json_writer_append_int32(&json_builder, result)) &&
          az_succeeded(az_json_writer_append_property_name(&json_builder, reported_description_property_name)) &&
          az_succeeded(az_json_writer_append_string(&json_builder, description)) &&
          az_succeeded(az_json_writer_append_property_name(&json_builder, reported_version_property_name)) &&
          az_succeeded(az_json_writer_append_int32(&json_builder, (INT)ack_version)) &&
          az_succeeded(az_json_writer_append_end_object(&json_builder)) &&
          az_succeeded(az_json_writer_append_end_object(&json_builder))))
    {
        printf("Failed build reported property with status message\r\n");
        return(NX_NOT_SUCCESSFUL);
    }

    if (component_name_ptr  != NX_NULL &&
        az_failed(az_json_writer_append_end_object(&json_builder)))
    {
        printf("Failed build reported property with status message\r\n");
        return(NX_NOT_SUCCESSFUL);
    }

    *byte_copied = (UINT)az_span_size(az_json_writer_get_bytes_used_in_destination(&json_builder));

    return(NX_AZURE_IOT_SUCCESS);
}
