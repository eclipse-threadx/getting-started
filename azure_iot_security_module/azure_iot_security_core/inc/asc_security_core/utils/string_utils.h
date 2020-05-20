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

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdint.h>
#include <stdbool.h>



/**
 * @brief   char* value getter
 *
 * @param   s    string
 *
 * @return char* value if is not NULL, empty otherwise
 */
const char* string_utils_value_or_empty(const char* s);


/**
 * @brief   Check if string is blank
 *
 * @param   s    string
 *
 * @return true iff string is blank
 */
bool string_utils_is_blank(const char* s);


#endif /* STRING_UTILS_H */