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

#ifndef STACK_H
#define STACK_H

#include "asc_security_core/utils/collection/collection.h"

#define STACK_DECLARATIONS(type) \
typedef struct stack_##type##_tag {\
    type* head;\
    size_t size;\
} stack_##type;\
typedef stack_##type* stack_##type##_handle;\
extern void stack_##type##_init(stack_##type##_handle stack);\
extern void stack_##type##_push(stack_##type##_handle stack, type* item);\
extern type* stack_##type##_pop(stack_##type##_handle stack);\
extern type* stack_##type##_peek(stack_##type##_handle stack);\

#define STACK_DEFINITIONS(type) \
extern void stack_##type##_init(stack_##type##_handle stack) {\
    if ((stack) == NULL) {\
        return;\
    }\
    (stack)->head = NULL;\
    (stack)->size = 0;\
}\
void stack_##type##_push(stack_##type##_handle stack, type* item) {\
    type* current_head = (stack)->head;\
    if ((current_head) == NULL) {\
        (stack)->head = item;\
        (item)->next = NULL;\
        (item)->previous = NULL;\
        (stack)->size++;\
        return;\
    }\
    (item)->next = current_head;\
    (item)->previous = NULL;\
    (current_head)->previous = item;\
    (stack)->head = item;\
    (stack)->size++;\
    return;\
}\
type* stack_##type##_pop(stack_##type##_handle stack) {\
    type* current_head = (stack)->head;\
    if ((current_head) == NULL) {\
        return NULL;\
    }\
    type* new_head = (current_head)->next;\
    (current_head)->previous = (current_head)->next = NULL;\
    if ((new_head) != NULL) {\
        (new_head)->previous = NULL;\
    }\
    (stack)->head = new_head;\
    return current_head;\
}\
type *stack_##type##_peek(stack_##type##_handle stack) {\
    return (stack)->head;\
}\

#endif /* STACK_H */