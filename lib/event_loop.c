#include <assert.h>
#include "event_loop.h"
#include "common.h"
#include "log.h"
#include "event_dispatcher.h"
#include "channel.h"
#include "utils.h"
#include "channel_map.h"

int handle_wakeup(void *data) {
    struct event_loop *ev_loop = (struct event_loop *) data;
    char one;
    ssize_t n = read(ev_loop->socket_pair[1], &one, sizeof(one));
    if (n != sizeof(one)) {
        perror("handle wakeup failed.");
    }

    x_msgx("wakeup, %s", ev_loop->thread_name);
}

struct event_loop *event_loop_init() {
    return event_loop_init_with_name(NULL);
}

struct event_loop *event_loop_init_with_name(char *thread_name) {
    struct event_loop *ev_loop = malloc(sizeof(struct event_loop));
    pthread_mutex_init(&ev_loop->mutex, NULL);
    pthread_cond_init(&ev_loop->cond, NULL);

    if (thread_name != NULL) {
        ev_loop->thread_name = thread_name;
    } else {
        ev_loop->thread_name = "main thread";
    }

    ev_loop->quit = 0;
    ev_loop->chan_map = malloc(sizeof(struct channel_map));
    map_init(ev_loop->chan_map);

#ifdef EPOLL_ENABLE
    ev_loop->ev_dispatcher = &epoll_dispatcher;
#else
    ev_loop->event_dispatcher = &poll_dispatcher;
#endif

    ev_loop->event_dispatcher_data = ev_loop->ev_dispatcher->init(ev_loop);

    ev_loop->owner_thread_id = pthread_self();
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, ev_loop->socket_pair) < 0) {
        perror("socketpair set failed.");
    }
    ev_loop->is_handle_pending = 0;
    ev_loop->pending_head = NULL;
    ev_loop->pending_tail = NULL;

    struct channel *chan = channel_new(ev_loop->socket_pair[1], EVENT_READ, handle_wakeup, NULL, ev_loop);
    event_loop_add_channel_event(ev_loop, ev_loop->socket_pair[1], chan);

    return ev_loop;
}

int event_loop_run(struct event_loop *ev_loop) {
    assert(ev_loop != NULL);

    struct event_dispatcher *dispatcher = ev_loop->ev_dispatcher;

    if (ev_loop->owner_thread_id != pthread_self()) {
        exit(1);
    }

    x_msgx("event loop run, %s", ev_loop->thread_name);

    struct timeval tv;
    tv.tv_sec = 1;

    while(!ev_loop->quit) {
        dispatcher->dispatch(ev_loop, &tv);

        event_loop_handle_pending_channel(ev_loop);
    }

    return 0;

}

void event_loop_wakeup(struct event_loop *ev_loop) {
    x_msgx("wakeup event loop, %s", ev_loop->thread_name);

    char one = 'a';
    ssize_t n = write(ev_loop->socket_pair[0], &one, sizeof(one));
    if (n != sizeof(one)) {
        perror("wakeup event loop thread failed.");
    }
}

int event_loop_handle_pending_channel(struct event_loop *ev_loop) {
    pthread_mutex_lock(&ev_loop->mutex);
    ev_loop->is_handle_pending = 1;

    struct channel_element *chan_elem = ev_loop->pending_head;
    while(chan_elem != NULL) {
        struct channel *chan = chan_elem->channel;
        int fd = chan->fd;
        if (chan_elem->type == 1) {
            event_loop_handle_pending_add(ev_loop, fd, chan);
        } else if (chan_elem->type == 2) {
            event_loop_handle_pending_remove(ev_loop, fd, chan);
        } else if (chan_elem->type == 3) {
            event_loop_handle_pending_update(ev_loop, fd, chan);
        }
        chan_elem = chan_elem->next;
    }

    ev_loop->pending_head = ev_loop->pending_tail = NULL;
    ev_loop->is_handle_pending = 0;

    pthread_mutex_unlock(&ev_loop->mutex);

    return 0;
}

void event_loop_channel_buffer_nolock(struct event_loop *ev_loop, int fd, struct channel *chan, int type) {
    struct channel_element *chan_elem = malloc(sizeof(struct channel_element));

    chan_elem->channel = chan;
    chan_elem->type = type;
    chan_elem->next = NULL;

    if (ev_loop->pending_head == NULL) {
        ev_loop->pending_head = ev_loop->pending_tail = chan_elem;
    } else {
        ev_loop->pending_tail->next = chan_elem;
        ev_loop->pending_tail = chan_elem;
    }
}

int event_loop_do_channel_event(struct event_loop *ev_loop, int fd, struct channel *chan, int type) {
    pthread_mutex_lock(&ev_loop->mutex);
    assert(ev_loop->is_handle_pending == 0);
    event_loop_channel_buffer_nolock(ev_loop, fd, chan, type);

    pthread_mutex_unlock(&ev_loop->mutex);
    if (!is_in_same_thread(ev_loop)) {
        event_loop_wakeup(ev_loop);
    } else {
        event_loop_handle_pending_channel(ev_loop);
    }
}

int event_loop_add_channel_event(struct event_loop *ev_loop, int fd, struct channel *chan) {
    return event_loop_do_channel_event(ev_loop, fd, chan, 1);
}

int event_loop_remove_channel_event(struct event_loop *ev_loop, int fd, struct channel *chan) {
    return event_loop_do_channel_event(ev_loop, fd, chan, 2);
}

int event_loop_update_channel_event(struct event_loop *ev_loop, int fd, struct channel *chan) {
    return event_loop_do_channel_event(ev_loop, fd, chan, 3);
}

int event_loop_handle_pending_add(struct event_loop *ev_loop, int fd, struct channel *chan) {
    x_msgx("add channel fd == %d, %s", fd, ev_loop->thread_name);

    struct channel_map *map = ev_loop->chan_map;

    if (fd < 0) {
        return 0;
    }

    if (fd >= map->nentries) {
        if (map_make_space(map, fd, sizeof(struct channel *)) == -1) {
            return -1;
        }
    }

    if (map->entries[fd] == NULL) {
        map->entries[fd] = chan;

        struct event_dispatcher *ev_dispatcher = ev_loop->ev_dispatcher;
        ev_dispatcher->add(ev_loop, chan);
        return 1;
    }

    return 0;
}

int event_loop_handle_pending_remove(struct event_loop *ev_loop, int fd, struct channel *chan) {
    struct channel_map *map = ev_loop->chan_map;
    assert(fd == chan->fd);

    if (fd < 0) {
        return 0;
    }

    if (fd >= map->nentries) {
        return -1;
    }

    struct channel *ch = map->entries[fd];

    struct event_dispatcher *ev_dispatcher = ev_loop->ev_dispatcher;

    int retval = 0;
    if (ev_dispatcher->del(ev_loop, ch) == -1) {
        retval = -1;
    } else {
        retval = 1;
    }

    map->entries[fd] = NULL;
    return retval;
}

int event_loop_handle_pending_update(struct event_loop *ev_loop, int fd, struct channel *chan) {
    struct channel_map *map = ev_loop->chan_map;

    if (fd < 0) {
        return 0;
    }

    if (map->entries[fd] == NULL) {
        return -1;
    }

    struct event_dispatcher *ev_dispatcher = ev_loop->ev_dispatcher;
    ev_dispatcher->update(ev_loop, chan);
}

int channel_event_activate(struct event_loop *ev_loop, int fd, int res) {
    struct channel_map *map = ev_loop->chan_map;

    if (fd < 0) {
        return 0;
    }

    if (fd >= map->nentries) {
        return -1;
    }

    struct channel *chan = map->entries[fd];
    assert(fd == chan->fd);

    if (res & EVENT_READ) {
        if (chan->ev_read_callback != NULL) {
            chan->ev_read_callback(chan->data);
        }
    }
    if (res & EVENT_WRITE) {
        if (chan->ev_write_callback) {
            chan->ev_write_callback(chan->data);
        }
    }

    return 0;
}