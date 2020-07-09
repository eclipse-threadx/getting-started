/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json_utils.h"

bool findJsonInt(const char* json, jsmntok_t* tokens, int tokens_count, const char* s, int* value)
{
    for (int i = 1; i < tokens_count; i++)
    {
        if ((tokens[i].type == JSMN_STRING) && (strlen(s) == tokens[i].end - tokens[i].start) &&
            (strncmp(json + tokens[i].start, s, tokens[i].end - tokens[i].start) == 0))
        {
            *value = atoi(json + tokens[i + 1].start);

            printf("Desired property %s = %d\r\n", s, *value);
            return true;
        }
    }

    return false;
}
