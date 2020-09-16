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

#ifndef NX_AZURE_IOT_PNP_HELPERS_H
#define NX_AZURE_IOT_PNP_HELPERS_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "azure/core/az_json.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_api.h"

/**
 * @brief Parse PnP command name
 *
 * @param[in] method_name_ptr Pointer to method name
 * @param[in] method_name_length Length of method name
 * @param[out] component_name_pptr Pointer to component pointer
 * @param[out] component_name_length_ptr Pointer to length of component name length
 * @param[out] pnp_command_name_pptr Pointer to command name pointer
 * @param[out] pnp_command_name_length_ptr Pointer to length of command name
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if successful parsed command name.
 */
UINT nx_azure_iot_pnp_helper_command_name_parse(UCHAR *method_name_ptr, UINT method_name_length,
                                                UCHAR **component_name_pptr, UINT *component_name_length_ptr,
                                                UCHAR **pnp_command_name_pptr, UINT *pnp_command_name_length_ptr);

/**
 * @brief Parse twin data and call callback on each desired property
 *
 * @param[in] packet_ptr `NX_PACKET` pointer containing the twin data
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
UINT nx_azure_iot_pnp_helper_twin_data_parse(NX_PACKET *packet_ptr, UINT is_partial,
                                             CHAR **sample_components_ptr, UINT sample_components_num,
                                             UCHAR *scratch_buf, UINT scratch_buf_len,
                                             VOID (*sample_desired_property_callback)(UCHAR *component_name_ptr,
                                                   UINT component_name_len, UCHAR *property_name_ptr,
                                                   UINT property_name_len,
                                                   az_json_reader property_value_reader, UINT version,
                                                   VOID *userContextCallback),
                                             VOID *context_ptr);

/**
 * @brief Create PnP telemetry message
 *
 * @param[in] iothub_client_ptr Pointer to `NX_AZURE_IOT_HUB_CLIENT`
 * @param[in] component_name Pointer to component name
 * @param[in] component_name_len Length of component name
 * @param[out] packet_pptr `NX_PACKET` return via the API.
 * @param[in] wait_option Ticks to wait if no packet is available.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if successful created NX_PACKET.
 */
UINT nx_azure_iot_pnp_helper_telemetry_message_create(NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                                      UCHAR *component_name, UINT component_name_len,
                                                      NX_PACKET **packet_pptr, UINT wait_option);

/**
 * @brief Build PnP reported property into user provided buffer
 *
 * @param[in] component_name_ptr Pointer to component name
 * @param[in] component_name_len Length of component name
 * @param[in] buffer_ptr Pointer to buffer used for storing message
 * @param[in] buffer_len Size of buffer
 * @param[out] data_copied_ptr Number of bytes copied into buffer
 * @param[in] append_reported_property Callback to add reported property
 * @param[in] context Context pass to callback
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if successful created reported property message.
 */
UINT nx_azure_iot_pnp_helper_build_reported_property(UCHAR *component_name_ptr, UINT component_name_len,
                                                     UINT (*append_reported_property)(az_json_writer *json_builder,
                                                                                      VOID *context),
                                                     VOID *context, UCHAR *buffer_ptr, UINT buffer_len,
                                                     UINT *data_copied_ptr);

/**
 * @brief Build reported property with status
 *
 * @param[in] component_name_ptr Pointer to component name
 * @param[in] component_name_len Length of component name
 * @param[in] property_name_ptr Pointer to property name
 * @param[in] property_name_len Length of property name
 * @param[in] append_value Callback to add property value
 * @param[in] context Context pass to callback
 * @param[in] result Status for reported property
 * @param[in] description Pointer to description
 * @param[in] description_len Length of description
 * @param[in] ack_version ack version
 * @param[in] buffer_ptr Pointer to buffer to where message is stored.
 * @param[in] buffer_len Length of buffer
 * @param[out] byte_copied Number of bytes copied to buffer
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if successful created reported property message.
 */
UINT nx_azure_iot_pnp_helper_build_reported_property_with_status(UCHAR *component_name_ptr, UINT component_name_len,
                                                                 UCHAR *property_name_ptr, UINT property_name_len,
                                                                 UINT (*append_value)(az_json_writer *builder,
                                                                                      VOID *context),
                                                                 VOID *context,
                                                                 INT result, UCHAR *description_ptr,
                                                                 UINT description_len,
                                                                 UINT ack_version, UCHAR *buffer_ptr,
                                                                 UINT buffer_len,
                                                                 UINT *byte_copied);

#ifdef __cplusplus
}
#endif
#endif /* NX_AZURE_IOT_PNP_HELPERS_H */