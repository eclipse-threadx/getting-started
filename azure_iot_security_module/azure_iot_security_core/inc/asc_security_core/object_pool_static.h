
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

#ifndef OBJECT_POOL_STATIC_H
#define OBJECT_POOL_STATIC_H

#include <stdbool.h>
#include <stdint.h>

#define OBJECT_POOL_DEFINITIONS(type, pool_size)\
STACK_DEFINITIONS(type);\
static bool _##type##_is_pool_initialized = false;\
static type _##type##_pool[pool_size];\
static stack_##type _stack_##type = {0};\
static stack_##type##_handle _stack_##type##_handle;\
void object_pool_##type##_init() {\
    if (_##type##_is_pool_initialized) {\
        return;\
    }\
\
    _stack_##type##_handle = &(_stack_##type);\
    stack_##type##_init(_stack_##type##_handle);\
    for (uint32_t i=0; i<pool_size; i++) {\
        type* object = _##type##_pool + i;\
        stack_##type##_push(_stack_##type##_handle, object);\
  }\
\
    _##type##_is_pool_initialized = true;\
}\
type* object_pool_##type##_get() {\
    object_pool_##type##_init();\
    return stack_##type##_pop(_stack_##type##_handle);\
}\
void object_pool_##type##_free(type* object) {\
    stack_##type##_push(_stack_##type##_handle, object);\
}\

#define OBJECT_POOL_DECLARATIONS(type, pool_size)\
STACK_DECLARATIONS(type);\
void object_pool_##type##_init();\
type* object_pool_##type##_get();\
void object_pool_##type##_free(type* object);\

#endif /* OBJECT_POOL_STATIC_H */