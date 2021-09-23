/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "nx_azure_iot_pnp_helpers.h"

#include <stdio.h>

#include "azure/core/az_json.h"
#include "nx_api.h"

/* Reported property response property keys.  */
static const CHAR sample_pnp_component_type_property_name[] = "__t";

/* Device twin keys */
static const CHAR sample_iot_hub_twin_desired_version[] = "$version";
static const CHAR sample_iot_hub_twin_desired[]         = "desired";

/* Move reader to the value of property name.  */
static UINT sample_json_child_token_move(
    NX_AZURE_IOT_JSON_READER* json_reader_ptr, UCHAR* property_name_ptr, UINT property_name_len)
{
    while (nx_azure_iot_json_reader_next_token(json_reader_ptr) == NX_AZURE_IOT_SUCCESS)
    {
        if ((nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME) &&
            nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr, property_name_ptr, property_name_len))
        {
            if (nx_azure_iot_json_reader_next_token(json_reader_ptr))
            {
                printf("Failed to read next token\r\n");
                return (NX_NOT_SUCCESSFUL);
            }

            return (NX_AZURE_IOT_SUCCESS);
        }
        else if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
        {
            if (nx_azure_iot_json_reader_skip_children(json_reader_ptr))
            {
                printf("Failed to skip child of complex object\r\n");
                return (NX_NOT_SUCCESSFUL);
            }
        }
        else if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
        {
            return (NX_AZURE_IOT_NOT_FOUND);
        }
    }

    return (NX_AZURE_IOT_NOT_FOUND);
}

/* Visit component property Object and call callback on each property of that component.  */
static UINT visit_component_properties(UCHAR* component_name_ptr,
    UINT component_name_len,
    NX_AZURE_IOT_JSON_READER* json_reader_ptr,
    UINT version,
    UCHAR* scratch_buf,
    UINT scratch_buf_len,
    VOID (*sample_desired_property_callback)(UCHAR* component_name_ptr,
        UINT component_name_len,
        UCHAR* property_name_ptr,
        UINT property_name_len,
        NX_AZURE_IOT_JSON_READER property_value_reader,
        UINT version,
        VOID* userContextCallback),
    VOID* context_ptr)
{
    UINT len;

    while (nx_azure_iot_json_reader_next_token(json_reader_ptr) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
        {
            if (nx_azure_iot_json_reader_token_string_get(json_reader_ptr, scratch_buf, scratch_buf_len, &len))
            {
                printf("Failed to get string property value\r\n");
                return (NX_NOT_SUCCESSFUL);
            }

            if (nx_azure_iot_json_reader_next_token(json_reader_ptr))
            {
                printf("Failed to get next token\r\n");
                return (NX_NOT_SUCCESSFUL);
            }

            if ((len == sizeof(sample_pnp_component_type_property_name) - 1) &&
                (memcmp((VOID*)scratch_buf, (VOID*)sample_pnp_component_type_property_name, len) == 0))
            {
                continue;
            }

            if ((len == sizeof(sample_iot_hub_twin_desired_version) - 1) &&
                (memcmp((VOID*)scratch_buf, (VOID*)sample_iot_hub_twin_desired_version, len) == 0))
            {
                continue;
            }

            sample_desired_property_callback(
                component_name_ptr, component_name_len, scratch_buf, len, *json_reader_ptr, version, context_ptr);
        }

        if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
        {
            if (nx_azure_iot_json_reader_skip_children(json_reader_ptr))
            {
                printf("Failed to skip children of object\r\n");
                return (NX_NOT_SUCCESSFUL);
            }
        }
        else if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
        {
            break;
        }
    }

    return (NX_AZURE_IOT_SUCCESS);
}

/* Check if component is part of component list.  */
static UINT is_component_in_model(UCHAR* component_name_ptr,
    UINT component_name_len,
    CHAR** sample_components_ptr,
    UINT sample_components_num,
    UINT* out_index)
{
    UINT index = 0;

    if (component_name_ptr == NX_NULL || component_name_len == 0)
    {
        return (NX_NOT_SUCCESSFUL);
    }

    while (index < sample_components_num)
    {
        if ((component_name_len == strlen(sample_components_ptr[index])) &&
            (memcmp((VOID*)component_name_ptr, (VOID*)sample_components_ptr[index], component_name_len) == 0))
        {
            *out_index = index;
            return (NX_AZURE_IOT_SUCCESS);
        }

        index++;
    }

    return (NX_AZURE_IOT_NOT_FOUND);
}

/* Parse twin data and call callback on each desired property.  */
UINT nx_azure_iot_pnp_helper_twin_data_parse(NX_AZURE_IOT_JSON_READER* json_reader_ptr,
    UINT is_partial,
    CHAR** sample_components_ptr,
    UINT sample_components_num,
    UCHAR* scratch_buf,
    UINT scratch_buf_len,
    VOID (*sample_desired_property_callback)(UCHAR* component_name_ptr,
        UINT component_name_len,
        UCHAR* property_name_ptr,
        UINT property_name_len,
        NX_AZURE_IOT_JSON_READER property_value_reader,
        UINT version,
        VOID* userContextCallback),
    VOID* context_ptr)
{
    NX_AZURE_IOT_JSON_READER copy_json_reader;
    UINT version;
    UINT len;
    UINT index;
    UINT status;

    if ((status = nx_azure_iot_json_reader_next_token(json_reader_ptr)))
    {
        printf("Failed to initialize json reader: error %d\r\n", status);
        return (status);
    }

    if (!is_partial &&
        sample_json_child_token_move(
            json_reader_ptr, (UCHAR*)sample_iot_hub_twin_desired, sizeof(sample_iot_hub_twin_desired) - 1))
    {
        printf("Failed to get desired property\r\n");
        return (NX_NOT_SUCCESSFUL);
    }

    copy_json_reader = *json_reader_ptr;
    if (sample_json_child_token_move(&copy_json_reader,
            (UCHAR*)sample_iot_hub_twin_desired_version,
            sizeof(sample_iot_hub_twin_desired_version) - 1) ||
        nx_azure_iot_json_reader_token_int32_get(&copy_json_reader, (int32_t*)&version))
    {
        printf("Failed to get version\r\n");
        return (NX_NOT_SUCCESSFUL);
    }

    while (nx_azure_iot_json_reader_next_token(json_reader_ptr) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
        {
            if (nx_azure_iot_json_reader_token_string_get(json_reader_ptr, scratch_buf, scratch_buf_len, &len))
            {
                printf("Failed to string value for property name\r\n");
                return (NX_NOT_SUCCESSFUL);
            }

            if (nx_azure_iot_json_reader_next_token(json_reader_ptr))
            {
                printf("Failed to next token\r\n");
                return (NX_NOT_SUCCESSFUL);
            }

            if ((len == sizeof(sample_iot_hub_twin_desired_version) - 1) &&
                (memcmp((VOID*)sample_iot_hub_twin_desired_version, (VOID*)scratch_buf, len) == 0))
            {
                continue;
            }

            if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT &&
                sample_components_ptr != NX_NULL &&
                (is_component_in_model(scratch_buf, len, sample_components_ptr, sample_components_num, &index) ==
                    NX_AZURE_IOT_SUCCESS))
            {
                if (visit_component_properties((UCHAR*)sample_components_ptr[index],
                        strlen(sample_components_ptr[index]),
                        json_reader_ptr,
                        version,
                        scratch_buf,
                        scratch_buf_len,
                        sample_desired_property_callback,
                        context_ptr))
                {
                    printf("Failed to visit component properties\r\n");
                    return (NX_NOT_SUCCESSFUL);
                }
            }
            else
            {
                sample_desired_property_callback(NX_NULL, 0, scratch_buf, len, *json_reader_ptr, version, context_ptr);

                if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
                {
                    if (nx_azure_iot_json_reader_skip_children(json_reader_ptr))
                    {
                        printf("Failed to skip children of object\r\n");
                        return (NX_NOT_SUCCESSFUL);
                    }
                }
            }
        }
        else if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
        {
            if (nx_azure_iot_json_reader_skip_children(json_reader_ptr))
            {
                printf("Failed to skip children of object\r\n");
                return (NX_NOT_SUCCESSFUL);
            }
        }
        else if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
        {
            break;
        }
    }

    return (NX_AZURE_IOT_SUCCESS);
}
