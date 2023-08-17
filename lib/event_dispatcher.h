#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H

#include <sys/time.h>
#include "event_loop.h"

struct event_loop;

struct event_dispatcher {
    const char *name;

    void *(*init)(struct event_loop *ev_loop);

    int  (*add) (struct event_loop *ev_loop, struct channel *chan);

    int (*del) (struct event_loop *ev_loop, struct channel *chan);

    int (*update) (struct event_loop *ev_loop, struct channel *chan);

    int (*dispatch)(struct event_loop * eventLoop, struct timeval * tv);

    void (*clean) (struct event_loop *ev_loop);
};

#endif