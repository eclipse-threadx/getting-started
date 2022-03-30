#ifndef GSG_LOG_H
#define GSG_LOG_H

#include <stdbool.h>
#include <stdint.h>

#define LOG_DISABLED 0
#define LOG_ERROR    1
#define LOG_WARNING  2
#define LOG_INFO     3
#define LOG_DEBUG    4

#define COMPONENT_NAME_SIZE 32
#define BUFFER_SIZE         128

typedef struct queue_buffer
{
    bool allocated;

    char component_name[COMPONENT_NAME_SIZE];
    uint32_t time;
    uint8_t log_level;
    char message[BUFFER_SIZE];

    struct queue_buffer* next;
} queue_buffer;

bool gsg_log_pop(queue_buffer** buffer_ptr);
bool gsg_log_unpop(queue_buffer* buffer_ptr);
void gsg_log_destroy(queue_buffer* buffer_ptr);
bool gsg_log_empty();
uint8_t gsg_log_size();
bool gsg_log_almost_full();
void gsg_log_level_set(uint8_t log_level);
void gsg_log(uint8_t log_level, char* component_name_ptr, char* message_ptr, ...);

#define GsgLogError(...)   gsg_log(LOG_ERROR, LOG_COMPONENT_NAME, __VA_ARGS__)
#define GsgLogWarning(...) gsg_log(LOG_WARNING, LOG_COMPONENT_NAME, __VA_ARGS__)
#define GsgLogInfo(...)    gsg_log(LOG_INFO, LOG_COMPONENT_NAME, __VA_ARGS__)
#define GsgLogDebug(...)   gsg_log(LOG_DEBUG, LOG_COMPONENT_NAME, __VA_ARGS__)

#endif