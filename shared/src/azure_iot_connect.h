/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_IOT_CONNECT_H
#define _AZURE_IOT_CONNECT_H

#include "nx_api.h"

#include "azure_iot_nx_client.h"

VOID connection_status_set(AZURE_IOT_NX_CONTEXT* nx_context, UINT connection_status);

VOID connection_monitor(
    AZURE_IOT_NX_CONTEXT* nx_context, UINT (*iothub_init)(AZURE_IOT_NX_CONTEXT* nx_context), UINT (*network_connect)());

#endif