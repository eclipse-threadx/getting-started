// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file _asc_cfg_prefix.h
 *
 * @brief Open "extern C" and save warnings state.
 *
 * NOTE: You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#ifdef _MSC_VER
#pragma warning(push)
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) // !_MSC_VER
#pragma GCC diagnostic push
#elif defined(__clang__) // !_MSC_VER !__clang__
#pragma clang diagnostic push
#endif // _MSC_VER

#include "asc_security_core/asc/_asc_cfg.h"
