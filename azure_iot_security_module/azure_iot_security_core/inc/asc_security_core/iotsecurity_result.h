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

#ifndef IOTSECURITY_RESULT_H
#define IOTSECURITY_RESULT_H

typedef enum IOTSECURITY_RESULT_TAG {
    IOTSECURITY_RESULT_OK                       = 0,
    IOTSECURITY_RESULT_EXCEPTION                = 1,
    IOTSECURITY_RESULT_MEMORY_EXCEPTION         = 2,
    IOTSECURITY_RESULT_NOT_SUPPORTED_EXCEPTION  = 3,
    IOTSECURITY_RESULT_RESOURCE_DESTROYED       = 4,
    IOTSECURITY_RESULT_TIMEOUT                  = 5,
    IOTSECURITY_RESULT_PENDING                  = 6,
    IOTSECURITY_RESULT_PARSE_EXCEPTION          = 7,
    IOTSECURITY_RESULT_OFF                      = 8,
    IOTSECURITY_RESULT_BAD_ARGUMENT             = 9,
    IOTSECURITY_RESULT_EMPTY                    = 10,
    IOTSECURITY_RESULT_FULL                     = 11,
    IOTSECURITY_RESULT_UNINITIALIZED            = 12,
    IOTSECURITY_RESULT_INITIALIZED              = 13,
} IOTSECURITY_RESULT;


#endif /* IOTSECURITY_RESULT_H */
