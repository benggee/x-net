#ifndef CHANNEL_H
#define CHANNEL_H

#define EVENT_TIMEOUT 0x01
#define EVENT_READ 0x02
#define EVENT_WRITE 0x04
#define EVENT_SIGNAL 0x08

typedef int (*event_read_callback)(void *data);

typedef int (*event_write_callback)(void *data);

struct channel {
    int fd;
    int events;

    event_read_callback ev_read_callback;
    event_write_callback ev_write_callback;
    void *data;
};

struct channel *channel_new(int fd, int events, event_read_callback ev_read_callback, event_write_callback ev_write_callback, void *data);

int channel_write_event_is_enabled(struct channel *chan);

int channel_write_event_enable(struct channel *chan);

int channel_write_event_disable(struct channel *chan);

#endif