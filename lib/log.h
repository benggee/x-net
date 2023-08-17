#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

#define LOG_DEBUG_TYPE 0
#define LOG_MSG_TYPE 1
#define LOG_WARN_TYPE 2
#define LOG_ERR_TYPE 3

void x_log(int serverity, const char *msg);
void x_logx(int serverity, const char *errstr, const char *fmt, va_list ap);
void x_msgx(const char *fmt, ...);
void x_debugx(const char *fmt, ...);

#define LOG_MSG(msg) x_log(LOG_MSG_TYPE, msg);
#define LOG_ERR(msg) x_log(LOG_ERR_TYPE, msg);

#endif