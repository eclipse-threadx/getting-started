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

#ifndef LOGGER_H
#define LOGGER_H

#define LOG_LEVEL_NOTSET    0
#define LOG_LEVEL_DEBUG     1
#define LOG_LEVEL_INFO      2
#define LOG_LEVEL_WARN      3
#define LOG_LEVEL_ERROR     4
#define LOG_LEVEL_FATAL     5


#if LOG_LEVEL == LOG_LEVEL_NOTSET
    #define log_debug(message, ...)
    #define log_info(message, ...)
    #define log_warn(message, ...)
    #define log_error(message, ...)
    #define log_fatal(message, ...)
#else
    #include <stdio.h>
    #include <string.h>
    #include <stdbool.h>
    #include <stdint.h>

    #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
    #define MDC_FORMAT " [%s/%s:%d] "

    bool logger_init();
    void logger_deinit();
    void logger_log(const char* message, ...);

    // define log by severity according to LOG_LEVEL
    #if LOG_LEVEL > LOG_LEVEL_DEBUG
        #define log_debug(message, ...)
    #else
        #define log_debug(message, ...)     logger_log("DEBUG" MDC_FORMAT message "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
    #endif
    #if LOG_LEVEL > LOG_LEVEL_INFO
        #define log_info(message, ...)
    #else
        #define log_info(message, ...)      logger_log("INFO " MDC_FORMAT message "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
    #endif
    #if LOG_LEVEL > LOG_LEVEL_WARN
        #define log_warn(message, ...)
    #else
        #define log_warn(message, ...)      logger_log("WARN " MDC_FORMAT message "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
    #endif
    #if LOG_LEVEL > LOG_LEVEL_ERROR
        #define log_error(message, ...)
    #else
        #define log_error(message, ...)     logger_log("ERROR" MDC_FORMAT message "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
    #endif
    #if LOG_LEVEL > LOG_LEVEL_FATAL
        #define log_fatal(message, ...)
    #else
        #define log_fatal(message, ...)     logger_log("FATAL" MDC_FORMAT message "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
    #endif
#endif


#endif //LOGGER_H