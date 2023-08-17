#include "common.h"
#include "log.h"
#include <stdarg.h>
#include <syslog.h>

void x_log(int serverity, const char *msg) {
    const char *serverity_str;
    switch (serverity) {
        case LOG_DEBUG_TYPE:
            serverity_str = "debug";
            break;
        case LOG_MSG_TYPE:
            serverity_str = "msg";
            break;
        case LOG_WARN_TYPE:
            serverity_str = "warn";
            break;
        case LOG_ERR_TYPE:
            serverity_str = "err";
            break;
        default:
            serverity_str = "unknow";
            break;
    }
    (void) fprintf(stdout, "[%s] %s\n", serverity_str, msg);
}
void x_logx(int serverity, const char *errstr, const char *fmt, va_list ap) {
    char buf[1024];
    size_t len;

    if (fmt != NULL) {
        vsnprintf(buf, sizeof(buf), fmt, ap);
    } else {
        buf[0] = '\0';
    }

    if (errstr) {
        len = strlen(buf);
        if (len < sizeof(buf) - 3) {
            snprintf(buf + len, sizeof(buf) - len, ": %s", errstr);
        }
    }

    x_log(serverity, buf);
}

void x_msgx(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    x_logx(LOG_MSG_TYPE, NULL, fmt, ap);
    va_end(ap);
}

void x_debugx(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    x_logx(LOG_DEBUG_TYPE, NULL, fmt, ap);
    va_end(ap);
}