#include "asc_security_core/utils/itime.h"

#define ISO8601_MAX_LENGTH 25
#define ISO8601_DATETIME_FORMAT "%FT%T"

static unix_time_callback_t _time_callback = NULL;

void itime_init(unix_time_callback_t time_callback) {
    _time_callback = time_callback;
}

time_t itime_time(time_t* timer) {
    if (_time_callback == NULL) {
        return (time_t)-1;
    }

    return _time_callback(timer);
}


struct tm* itime_utcnow(time_t* timer, struct tm* buf) {
#ifndef ASC_TIME_IAR
#ifdef _WIN32
    return gmtime_s(buf, timer) == 0 ? buf : NULL;
#else // _WIN32
    return gmtime_r(timer, buf);
#endif // _WIN32
#else // ASC_TIME_IAR
    return gmtime_s(timer, buf);
#endif // ASC_TIME_IAR
}


struct tm* itime_localtime(time_t* timer, struct tm* buf) {
#ifndef ASC_TIME_IAR
#ifdef _WIN32
    return localtime_s(buf, timer) == 0 ? buf: NULL;
#else // _WIN32
    return localtime_r(timer, buf);
#endif // _WIN32
#else // ASC_TIME_IAR
    return localtime_s(timer, buf);
#endif // ASC_TIME_IAR
}


size_t itime_iso8601(const struct tm* tp, char* s) {
    return strftime(s, ISO8601_MAX_LENGTH, ISO8601_DATETIME_FORMAT, tp);
}


double itime_difftime(time_t time1, time_t time0) {
    return difftime(time1, time0);
}
