// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file asc_precondition.h
 *
 * @brief This header defines the types and functions your application uses
 *        to override the default precondition failure behavior.
 *
 *        Public SDK functions validate the arguments passed to them in an effort
 *        to ensure that calling code is passing valid values. The valid value is
 *        called a contract precondition. If an SDK function detects a precondition
 *        failure (invalid argument value), then by default, it calls a function that
 *        places the calling thread into an infinite sleep state; other threads
 *        continue to run.
 *
 *        To override the default behavior, implement a function matching the
 *        asc_precondition_failed_fn function signature and then, in your application's
 *        initialization (before calling any Azure SDK function), call
 *        asc_precondition_failed_set_callback passing it the address of your function.
 *        Now, when any Azure SDK function detects a precondition failure, it will invoke
 *        your callback instead. You might override the callback to attach a debugger or
 *        perhaps to reboot the device rather than allowing it to continue running with
 *        unpredictable behavior.
 *
 *        Also, if you define the NO_PRECONDITION_CHECKING symbol when compiling the SDK
 *        code, all of the Azure SDK precondition checking will be excluding making the
 *        binary code smaller and faster. We recommend doing this before you ship your code.
 *
 * NOTE: You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _asc_PRECONDITION_H
#define _asc_PRECONDITION_H

#include <stdbool.h>
#include <stddef.h>

#include "asc_security_core/asc/asc_span.h"

#include "asc_security_core/asc/_asc_cfg_prefix.h"

/**
 * @brief asc_precondition_failed_fn defines the signature of the callback function that application
 * developers can write in order to override the default precondition failure behavior.
 */
typedef void (*asc_precondition_failed_fn)();

/**
 * @brief  asc_precondition_failed_set_callback allows your application to override the default
 * behavior in response to an SDK function detecting an invalid argument (precondition failure).
 * Call this function once when your application initializes and before you call and Azure SDK
 * functions.
 *
 * @param asc_precondition_failed_callback A pointer to the function that will be invoked when
 *                                        an Azure SDK function detects a precondition failure.
 */
void asc_precondition_failed_set_callback(asc_precondition_failed_fn asc_precondition_failed_callback);

#include "asc_security_core/asc/_asc_cfg_suffix.h"

#endif // _asc_PRECONDITION_H
