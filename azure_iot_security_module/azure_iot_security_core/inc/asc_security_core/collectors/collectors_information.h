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

#ifndef _COLLECTORS_INFORMATION_H_
#define _COLLECTORS_INFORMATION_H_

#include <stdint.h>
#include "asc_security_core/asc/asc_span.h"

typedef intptr_t collectors_info_handle;
/**
 * @brief Initialize collectors info module
 *
 * @return Collectors info data struct handler
 */
collectors_info_handle collectors_info_init();

/**
 * @brief Deinitialize collectors info module
 *
 * @param collectors_info   collectors_info_handle
 *
 * @return None
 */
void collectors_info_deinit(collectors_info_handle collectors_info);

/**
 * @brief Append collectors info to pairs struct
 *
 * @param collectors_info   system_information_collectors_info_handle
 * @param pairs             pairs struct
 * @param max_pairs         length of pairs array
 *
 * @return None
 */
void collectors_info_append(collectors_info_handle collectors_info, asc_pair* pairs, int max_pairs);

#endif /* _COLLECTORS_INFORMATION_H_ */
