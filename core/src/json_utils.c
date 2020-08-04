/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json_utils.h"

bool findJsonInt(const char* json, jsmntok_t* tokens, int tokens_count, const char* s, int* value)
{
    for (int i = 0; i < tokens_count - 1; i++)
    {
        if (tokens[i].type == JSMN_STRING && tokens[i + 1].type == JSMN_PRIMITIVE)
        {
            if ((strlen(s) == tokens[i].end - tokens[i].start) &&
                (strncmp(json + tokens[i].start, s, tokens[i].end - tokens[i].start) == 0))
            {
                *value = atoi(json + tokens[i + 1].start);

                printf("Writeable property %s = %d\r\n", s, *value);
                return true;
            }
        }
    }

    return false;
}
