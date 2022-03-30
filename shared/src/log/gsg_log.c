
#include "gsg_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "sntp_client.h"

#define LOG_COMPONENT_NAME "gsg_log"

#define BUFFER_COUNT 30

static queue_buffer buffer_pool[BUFFER_COUNT] = {0};
static char temp_buffer[BUFFER_SIZE];

static queue_buffer* buffer_start = 0;
static queue_buffer* buffer_end   = 0;
static uint8_t buffer_size        = 0;
static uint8_t gsg_log_level      = LOG_ERROR;

static int url_encode(char* dest, uint16_t dest_size, char* msg)
{
    const char* hex = "0123456789abcdef";
    char* startPtr  = dest;

    while (*msg != 0)
    {
        if (('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z') || ('0' <= *msg && *msg <= '9'))
        {
            *dest++ = *msg;
        }
        else
        {
            *dest++ = '%';
            *dest++ = hex[*msg >> 4];
            *dest++ = hex[*msg & 15];
        }
        if (--dest_size < 4)
        {
            printf("ERROR url_encode buffer filled\r\n");
        }

        msg++;

    }
    *dest = 0; // null terminator
    return dest - startPtr;
}

// Fetch a free buffer from the pool
static bool gsg_log_create(queue_buffer** buffer_ptr)
{
    for (int i = 0; i < BUFFER_COUNT; ++i)
    {
        if (buffer_pool[i].allocated == false)
        {
            buffer_pool[i].allocated = true;
            *buffer_ptr              = &buffer_pool[i];
            return true;
        }
    }

    // Pop an item off and reuse that buffer
    printf("ERROR no empty log buffers, reusing\r\n");
    gsg_log_pop(buffer_ptr);
    return true;
}

// Add the buffer to the queue
static void gsg_log_push(queue_buffer* buffer_ptr)
{
    if (buffer_size == 0)
    {
        // First item in the queue
        buffer_start = buffer_ptr;
        buffer_end   = buffer_ptr;
    }
    else
    {
        // Add to the end of the queue
        buffer_end->next = buffer_ptr;
        buffer_end       = buffer_ptr;
    }

    buffer_size++;
}

bool gsg_log_pop(queue_buffer** buffer_ptr)
{
    if (buffer_size == 0)
    {
        // No items in the list
        return false;
    }

    *buffer_ptr = buffer_start;

    if (buffer_start->next != 0)
    {
        // Otherwise advance the queue to the next item
        buffer_start = (*buffer_ptr)->next;
    }

    // Disconnect the returned item from the queue
    (*buffer_ptr)->next = 0;
    buffer_size--;
    return true;
}

bool gsg_log_unpop(queue_buffer* buffer_ptr)
{
    if (buffer_size == 0)
    {
        // First item in the queue
        buffer_start = buffer_ptr;
        buffer_end   = buffer_ptr;
    }
    else
    {
        // Add to the end of the queue
        buffer_end->next = buffer_ptr;
        buffer_end       = buffer_ptr;
    }

    buffer_size++;
}

// Free the buffer
void gsg_log_destroy(queue_buffer* buffer_ptr)
{
    buffer_ptr->allocated = false;
}

bool gsg_log_empty()
{
    return buffer_size == 0;
}

uint8_t gsg_log_size()
{
    return buffer_size;
}

bool gsg_log_almost_full()
{
    return (buffer_size + 5) > BUFFER_COUNT;
}

void gsg_log_level_set(uint8_t log_level)
{
    gsg_log_level = log_level;
}

void gsg_log(uint8_t log_level, char* component_name_ptr, char* message_ptr, ...)
{
    va_list args;
    queue_buffer* buffer_ptr;

    if (!gsg_log_create(&buffer_ptr))
    {
        printf("ERROR: gsg_log failed to get buffer\r\n");
        return;
    }

    if (strlen(component_name_ptr) > COMPONENT_NAME_SIZE - 1)
    {
        printf("ERROR: component_name_ptr is too long: %s\r\n", component_name_ptr);
        return;
    }

    strcpy(buffer_ptr->component_name, component_name_ptr);
    buffer_ptr->log_level = log_level;
    buffer_ptr->time      = sntp_time_get();

    va_start(args, message_ptr);
    int bytes_written = vsnprintf(temp_buffer, BUFFER_SIZE, message_ptr, args);
    va_end(args);

    url_encode(buffer_ptr->message, BUFFER_SIZE, temp_buffer);

    // output to console
    printf("[%ld][%d][%s] %s\r\n", buffer_ptr->time, buffer_ptr->log_level, buffer_ptr->component_name, buffer_ptr->message);

    if (bytes_written <= 0 || bytes_written >= BUFFER_SIZE)
    {
        GsgLogError("gsg_log truncation of size %d", bytes_written);
    }

    gsg_log_push(buffer_ptr);
}
