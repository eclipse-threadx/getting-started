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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "asc_security_core/utils/string_utils.h"


const char* string_utils_value_or_empty(const char* s) {
    return (s == NULL) ? "" : s;
}


bool string_utils_is_blank(const char* s) {
    if (s == NULL) {
        return true;
    }

    while (*s != '\0') {
        if (!isspace((unsigned char)*s)) {
            return false;
        }

        s++;
    }

    return true;
}