
#include "channel.h"

struct channel *channel_new(int fd, int events, event_read_callback ev_read_callback, event_write_callback ev_write_callback, void *data) {
    struct channel *chan = malloc(sizeof(struct channel));
    if (!chan) {
        return NULL;
    }

    chan->fd = fd;
    chan->events = events;
    chan->ev_read_callback = ev_read_callback;
    chan->ev_write_callback = ev_write_callback;
    chan->data = data;

    return chan;
}

int channel_write_event_is_enabled(struct channel *chan) {
    if (!chan) {
        return 0;
    }
    return chan->events & EVENT_WRITE;
}

int channel_write_event_enable(struct channel *chan) {
    if (!chan) {
        return -1;
    }

    struct event_loop *ev_loop = (struct event_loop *) chan->data;
    chan->events = chan->events | EVENT_WRITE;
    event_loop_update_channel_event(ev_loop, chan->fd, chan);

    return 0;
}

int channel_write_event_disable(struct channel *chan) {
    if (!chan) {
        return -1;
    }

    struct event_loop *ev_loop = (struct event_loop *) chan->data;
    chan->events = chan->events & ~EVENT_WRITE;
    event_loop_update_channel_event(ev_loop, chan->fd, chan);

    return 0;
}