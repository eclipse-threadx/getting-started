// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "az_ulib_dm.h"
#include "cipher_v1i1.h"
#include "cipher_v2i1.h"

#include <string.h>

az_result az_ulib_dm_install(char* lib, unsigned short length)
{
    az_result result;

    if(strncmp(lib, "\"cipher_v1i1\"", length) == 0)
    {
        result = cipher_v1i1_create();
    }
    else if(strncmp(lib, "\"cipher_v2i1\"", length) == 0)
    {
        result = cipher_v2i1_create();
    }
    else
    {
        result = AZ_ERROR_ITEM_NOT_FOUND;
    }

    return result;
}

az_result az_ulib_dm_uninstall(char* lib, unsigned short length)
{
    az_result result;

    if(strncmp(lib, "\"cipher_v1i1\"", length) == 0)
    {
        result = cipher_v1i1_destroy();
    }
    else if(strncmp(lib, "\"cipher_v2i1\"", length) == 0)
    {
        result = cipher_v2i1_destroy();
    }
    else
    {
        result = AZ_ERROR_ITEM_NOT_FOUND;
    }

    return result;
}
