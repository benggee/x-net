#include <assert.h>
#include "event_loop_thread.h"
#include "event_loop.h"

void *event_loop_thread_run(void *arg) {
    struct event_loop_thread *ev_loop_thread = (struct event_loop_thread *) arg;

    pthread_mutex_lock(&ev_loop_thread->mutex);

    ev_loop_thread->ev_loop = event_loop_init_with_name(ev_loop_thread->thread_name);
    pthread_cond_signal(&ev_loop_thread->cond);

    pthread_mutex_unlock(&ev_loop_thread->mutex);

    event_loop_run(ev_loop_thread->ev_loop);
}

int event_loop_thread_init(struct event_loop_thread *ev_loop_thread, int i) {
    pthread_mutex_init(&ev_loop_thread->mutex, NULL);
    pthread_cond_init(&ev_loop_thread->cond, NULL);

    ev_loop_thread->ev_loop = NULL;
    ev_loop_thread->thread_count = 0;
    ev_loop_thread->thread_id = 0;

    char *buf = malloc(16);
    ev_loop_thread->thread_name = buf;

    return 0;
}

struct event_loop *event_loop_thread_start(struct event_loop_thread *ev_loop_thread) {
    pthread_create(&ev_loop_thread->thread_id, NULL, &event_loop_thread_run, ev_loop_thread);

    assert(pthread_mutex_lock(&ev_loop_thread->mutex) == 0);

    while(ev_loop_thread->ev_loop == NULL) {
        assert(pthread_cond_wait(&ev_loop_thread->cond, &ev_loop_thread->mutex) == 0);
    }

    assert(pthread_mutex_unlock(&ev_loop_thread->mutex) == 0);

    return ev_loop_thread->ev_loop;
}