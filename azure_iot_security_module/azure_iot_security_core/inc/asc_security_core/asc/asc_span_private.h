// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _asc_SPAN_PRIVATE_H
#define _asc_SPAN_PRIVATE_H

#include "asc_security_core/asc/asc_precondition.h"
#include "asc_security_core/asc/asc_precondition_internal.h"
#include "asc_security_core/asc/asc_result.h"
#include "asc_security_core/asc/asc_span.h"

#include <stdbool.h>

#include "asc_security_core/asc/_asc_cfg_prefix.h"

/**
 * @brief Use this only to create a span from uint8_t object.
 * The size of the returned span is always one.
 * Don't use this function for arrays. Use @var ASC_SPAN_FROM_ARRAY instead.
 * Don't us
 */
ASC_NODISCARD ASC_INLINE asc_span asc_span_from_single_item(uint8_t* ptr)
{
  return asc_span_init(ptr, 1, 1);
}

/**
 * @brief Replace all contents from a starting position to an end position with the content of a
 * provided span
 *
 * @param self src span where to replace content
 * @param start starting position where to replace
 * @param end end position where to replace
 * @param span content to use for replacement
 * @return ASC_NODISCARD asc_span_replace
 */
ASC_NODISCARD asc_result _asc_span_replace(asc_span* self, int32_t start, int32_t end, asc_span span);

typedef asc_result (*_asc_predicate)(asc_span slice);

// PRIVATE. read until condition is true on character.
// Then return number of positions read with output parameter
ASC_NODISCARD asc_result _asc_scan_until(asc_span self, _asc_predicate predicate, int32_t* out_index);

ASC_NODISCARD asc_result _asc_is_expected_span(asc_span* self, asc_span expected);

#include "asc_security_core/asc/_asc_cfg_suffix.h"

#endif // _asc_SPAN_PRIVATE_H
