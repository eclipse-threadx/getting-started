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


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   Utility                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "nx_api.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_utility_string_length_check                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function traverses the string and returns the string length,   */
/*    if the string is invalid or the string length is bigger than max    */
/*    string length, returns error.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    input_string                          Pointer to input string       */
/*    string_length                         Pointer to string length      */
/*    max_string_length                     Max string length             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_utility_string_length_check(CHAR *input_string, UINT *string_length, UINT max_string_length)
{

UINT    i;


    /* Check for invalid input pointers.  */
    if (input_string == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Traverse the string.  */
    for (i = 0; input_string[i]; i++)
    {

        /* Check if the string length is bigger than the max string length.  */
        if (i >= max_string_length)
        {
            return(NX_SIZE_ERROR);
        }
    }

    /* Return the string length if string_length is not NULL.
       String_length being NULL indicates the caller needs to check for string 
       length within the max_string_length. */
    if (string_length)
    {
        *string_length = i;
    }

    /* Return success.  */
    return(NX_SUCCESS);
}

