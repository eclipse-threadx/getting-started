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

#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include "asc_security_core/utils/collection/collection.h"
#include "asc_security_core/utils/collection/stack.h"
#include "asc_security_core/configuration.h"

#ifdef USE_OBJECT_POOL
#include "asc_security_core/object_pool_static.h"
#else
#include "asc_security_core/object_pool_dynamic.h"
#endif

#define object_pool_get(type)              object_pool_##type##_get()
#define object_pool_free(type,object)      object_pool_##type##_free(object)

#endif /* OBJECT_POOL_H */