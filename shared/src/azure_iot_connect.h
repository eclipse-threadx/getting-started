/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_IOT_CONNECT_H
#define _AZURE_IOT_CONNECT_H

#include "nx_api.h"

VOID connection_monitor(AZURE_IOT_NX_CONTEXT* nx_context, UINT (*iothub_init)(AZURE_IOT_NX_CONTEXT* nx_context));

#endif