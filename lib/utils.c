#include "utils.h"
#include "log.h"
#include "common.h"

void assert_in_same_thread(struct event_loop *ev_loop) {
    if (ev_loop->owner_thread_id != pthread_self()) {
        LOG_ERR("not int the same thread")
        exit(-1);
    }
}

int is_in_same_thread(struct event_loop *ev_loop) {
    return ev_loop->owner_thread_id == pthread_self();
}

void *memmemx(const void *haystack, size_t haystack_len,
                         const void *needle, size_t needle_len) {
    if (needle_len == 0) {
        return (void *)haystack;
    }

    if (haystack_len < needle_len) {
        return NULL;
    }

    const char *haystack_char = (const char *)haystack;
    const char *needle_char = (const char *)needle;
    void *result = memchr(haystack_char, needle_char[0], haystack_len);
    while (result != NULL) {
        if (strncmp(result, needle_char, needle_len) == 0) {
            // It's a match!
            return result;
        }

        haystack_char = result + 1;
        haystack_len = haystack_len - (haystack_char - (const char *)haystack);
        result = memchr(haystack_char, needle_char[0], haystack_len);
    }

    return NULL;
}