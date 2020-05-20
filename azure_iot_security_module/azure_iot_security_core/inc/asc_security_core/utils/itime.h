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

#ifndef ITIME_H
#define ITIME_H

#include <time.h>

typedef time_t (*unix_time_callback_t)(time_t *unix_time);

void itime_init(unix_time_callback_t time_callback);

/**
 * @brief Get current time and put it in *TIMER if TIMER is not NULL
 *
 * @details This function provides the same functionality as the
 *          standard C @c time() function.
 */
time_t itime_time(time_t* timer);


/**
 * @brief Get current time and put it in timer if timer is not NULL.
 *
 * @details This function provides the same functionality as the
 *          standard C @c gmtime() function.
 */
struct tm* itime_utcnow(time_t* timer, struct tm* buf);


/**
 * @brief Get current time in the local timezone and put it in timer if timer is not NULL.
 *
 * @details This function provides the same functionality as the
 *          standard C @c localtime() function.
 */
struct tm* itime_localtime(time_t* timer, struct tm* buf);


/**
 * @brief Format TP into S according to ISO 8601
 *
 * @return size of S
 */
size_t itime_iso8601(const struct tm* tp, char* s);


/**
 * @brief Calculate the difference between TIME1 and TIME0
 *
 * @return time difference
 */
double itime_difftime(time_t time1, time_t time0);

#endif /* ITIME_H */