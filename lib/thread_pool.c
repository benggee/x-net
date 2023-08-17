#include <assert.h>
#include "thread_pool.h"
#include "utils.h"


struct thread_pool *thread_pool_new(struct event_loop *main_loop, int thread_number) {
    struct thread_pool *th_pool = malloc(sizeof(struct thread_pool));
    th_pool->main_loop = main_loop;
    th_pool->position = 0;
    th_pool->thread_number = thread_number;
    th_pool->started = 0;
    th_pool->ev_loop_threads = NULL;
    return th_pool;
}

void thread_pool_start(struct thread_pool *th_pool) {
    assert(!th_pool->started);
    assert_in_same_thread(th_pool->main_loop);

    th_pool->started = 1;
    void *tmp;

    if (th_pool->thread_number <= 0) {
        return;
    }

    th_pool->ev_loop_threads = malloc(th_pool->thread_number * sizeof(struct event_loop_thread));
    for (int i = 0; i < th_pool->thread_number; ++i) {
        event_loop_thread_init(&th_pool->ev_loop_threads[i], i);
        event_loop_thread_start(&th_pool->ev_loop_threads[i]);
    }
}

struct event_loop *thread_pool_get_loop(struct thread_pool *th_pool) {
    assert(th_pool->started);
    assert_in_same_thread(th_pool->main_loop);

    struct event_loop *selected = th_pool->main_loop;

    if (th_pool->thread_number > 0) {
        selected = th_pool->ev_loop_threads[th_pool->position].ev_loop;
        if (++th_pool->position >= th_pool->thread_number) {
            th_pool->position = 0;
        }
    }

    return selected;
}