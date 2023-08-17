#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H

#include <pthread.h>

struct event_loop_thread {
    struct event_loop *ev_loop;
    pthread_t thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char *thread_name;
    long thread_count;
};

int event_loop_thread_init(struct event_loop_thread *ev_loop_thread, int i);

struct event_loop *event_loop_thread_start(struct event_loop_thread *ev_loop_thread);

#endif