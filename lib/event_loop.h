#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <pthread.h>
#include "channel.h"
#include "event_dispatcher.h"
#include "common.h"

extern const struct event_dispatcher epoll_dispatcher;

struct channel_element {
    int type;
    struct channel *channel;
    struct channel_element *next;
};

struct event_loop {
    int quit;
    const struct event_dispatcher *ev_dispatcher;

    void *event_dispatcher_data;
    struct channel_map *chan_map;

    int is_handle_pending;
    struct channel_element *pending_head;
    struct channel_element *pending_tail;

    pthread_t owner_thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int socket_pair[2];
    char *thread_name;
};

struct event_loop *event_loop_init();

struct event_loop *event_loop_init_with_name(char *thread_name);

int event_loop_run(struct event_loop *ev_loop);

void event_loop_wakeup(struct event_loop *ev_loop);

int event_loop_add_channel_event(struct event_loop *ev_loop, int fd, struct channel *chan);

int event_loop_remove_channel_event(struct event_loop *ev_loop, int fd, struct channel *chan);

int event_loop_update_channel_event(struct event_loop *ev_loop, int fd, struct channel *chan);

int event_loop_handle_pending_add(struct event_loop *ev_loop, int fd, struct channel *chan);

int event_loop_handle_pending_remove(struct event_loop *ev_loop, int fd, struct channel *chan);

int event_loop_handle_pending_update(struct event_loop *ev_loop, int fd, struct channel *chan);

int channel_event_activate(struct event_loop *ev_loop, int fd, int res);

int event_loop_handle_pending_channel(struct event_loop *ev_loop);

#endif