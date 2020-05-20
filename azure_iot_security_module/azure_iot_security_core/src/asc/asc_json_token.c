// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "asc_security_core/asc/asc_json.h"
#include "asc_security_core/asc/asc_precondition_internal.h"

#include "asc_security_core/asc/_asc_cfg.h"

ASC_NODISCARD asc_result asc_json_token_get_boolean(asc_json_token const*  token, bool* out_value)
{

////  ASC_PRECONDITION_NOT_NULL(out_value);

  if (token->kind != ASC_JSON_TOKEN_BOOLEAN)
  {
    return ASC_ERROR_ITEM_NOT_FOUND;
  }

  *out_value = token->_internal.boolean;
  return ASC_OK;
}

ASC_NODISCARD asc_result asc_json_token_get_string(asc_json_token const*  token, asc_span* out_value)
{

////  ASC_PRECONDITION_NOT_NULL(out_value);

  if (token->kind != ASC_JSON_TOKEN_STRING)
  {
    return ASC_ERROR_ITEM_NOT_FOUND;
  }

  *out_value = token->_internal.string;
  return ASC_OK;
}

ASC_NODISCARD asc_result asc_json_token_get_number(asc_json_token const* token, double* out_value)
{

////  ASC_PRECONDITION_NOT_NULL(out_value);

  if (token->kind != ASC_JSON_TOKEN_NUMBER)
  {
    return ASC_ERROR_ITEM_NOT_FOUND;
  }

  *out_value = token->_internal.number;
  return ASC_OK;
}
