#ifndef UTILS_H
#define UTILS_H

#include "event_loop.h"

void assertInSameThread(struct event_loop *ev_loop);

int isInSameThread(struct event_loop *ev_loop);

void *memmemx(const void *haystack, size_t haystack_len,
              const void *needle, size_t needle_len);

#endif