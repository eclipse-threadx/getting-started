/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef NX_AZURE_IOT_PNP_HELPERS_H
#define NX_AZURE_IOT_PNP_HELPERS_H

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_json_reader.h"
#include "nx_azure_iot_json_writer.h"

/**
 * @brief Parse twin data and call callback on each desired property
 *
 * @param[in] json_reader_ptr `NX_AZURE_IOT_JSON_READER` pointer containing the twin data
 * @param[in] is_partial 1 if twin data is patch else 0 if full twin document
 * @param[in] sample_components_ptr Pointer to list of all components name pointers
 * @param[in] sample_components_num Size of component list
 * @param[in] scratch_buf Temporary buffer used for staging property names out of the JSON document
 * @param[in] scratch_buf_len Temporary buffer length size
 * @param[in] sample_desired_property_callback Callback called with each desired property
 * @param[in] context_ptr Context passed to the callback
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if successful parsed twin data.
 */
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
    VOID* context_ptr);

#endif
