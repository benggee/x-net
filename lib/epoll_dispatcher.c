#include <sys/epoll.h>
#include "event_loop.h"
#include "event_dispatcher.h"
#include <errno.h>
#include "log.h"

#define MAXEVENTS 128

typedef struct {
    int event_count;
    int nfds;
    int realloc_copy;
    int efd;
    struct epoll_event *events;
} epoll_dispatcher_data;

static void *epoll_init(struct event_loop *);

static int epoll_add(struct event_loop *, struct channel *chan);

static int epoll_del(struct event_loop *, struct channel *chan);

static int epoll_update(struct event_loop *, struct channel *chan);

static int epoll_dispatch(struct event_loop *, struct timeval *tv);

static void epoll_clear(struct event_loop *);

const struct event_dispatcher epoll_dispatcher = {
        "epoll",
        epoll_init,
        epoll_add,
        epoll_del,
        epoll_update,
        epoll_dispatch,
        epoll_clear,
};

void *epoll_init(struct event_loop *ev_loop) {
    epoll_dispatcher_data *ep_dis_data = malloc(sizeof(epoll_dispatcher_data));

    ep_dis_data->event_count = 0;
    ep_dis_data->nfds = 0;
    ep_dis_data->realloc_copy = 0;
    ep_dis_data->efd = 0;

    ep_dis_data->efd = epoll_create1(0);
    if (ep_dis_data->efd == -1) {
        perror("epoll_create1");
    }

    ep_dis_data->events = calloc(MAXEVENTS, sizeof(struct epoll_event));

    return ep_dis_data;
}

int epoll_add(struct event_loop *ev_loop, struct channel *chan) {
    epoll_dispatcher_data *ep_dis_data = (epoll_dispatcher_data *) ev_loop->event_dispatcher_data;

    int fd = chan->fd;
    int events = 0;
    if (chan->events & EVENT_READ) {
        events = events | EPOLLIN;
    }
    if (chan->events & EVENT_WRITE) {
        events = events | EPOLLOUT;
    }

    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;

    x_msgx("epoll add fd %d events read %d write %d", fd, chan->events & EVENT_READ, chan->events & EVENT_WRITE);

    if (epoll_ctl(ep_dis_data->efd, EPOLL_CTL_ADD, fd, &event) == -1) {
        perror("epoll ctl add failed.");
    }

    return 0;
}

int epoll_del(struct event_loop *ev_loop, struct channel *chan) {
    epoll_dispatcher_data *ep_dis_data = (epoll_dispatcher_data *) ev_loop->event_dispatcher_data;

    int fd = chan->fd;

    int events = 0;
    if (chan->events & EVENT_READ) {
        events = events | EPOLLIN;
    }
    if (chan->events & EVENT_WRITE) {
        events = events | EPOLLOUT;
    }

    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;

    if (epoll_ctl(ep_dis_data->efd, EPOLL_CTL_DEL, fd, &event) == -1) {
        perror("epoll ctl del failed.");
    }

    return 0;
}

int epoll_update(struct event_loop *ev_loop, struct channel *chan) {
    epoll_dispatcher_data *ep_dis_data = (epoll_dispatcher_data *) ev_loop->event_dispatcher_data;

    int fd = chan->fd;

    int events = 0;
    if (chan->events & EVENT_READ) {
        events = events | EPOLLIN;
    }
    if (chan->events & EVENT_WRITE) {
        events = events | EPOLLOUT;
    }

    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;

    if (epoll_ctl(ep_dis_data->efd, EPOLL_CTL_MOD, fd, &event) == -1) {
        perror("epoll ctl mod failed.");
    }

    return 0;
}

int epoll_dispatch(struct event_loop *ev_loop, struct timeval *tv) {
    epoll_dispatcher_data *ep_dis_data = (epoll_dispatcher_data *) ev_loop->event_dispatcher_data;

    int i, n;

    n = epoll_wait(ep_dis_data->efd, ep_dis_data->events, MAXEVENTS, -1);

    x_msgx("epoll_wait wakeup, %s", ev_loop->thread_name);
    for (i = 0; i < n; i++) {
        if ((ep_dis_data->events[i].events & EPOLLERR) || (ep_dis_data->events[i].events & EPOLLHUP)) {
            perror("epoll error");
            close(ep_dis_data->events[i].data.fd);
            continue;
        }

        if (ep_dis_data->events[i].events & EPOLLIN) {
            x_msgx("get message channel fd:%d, fro read, %s", ep_dis_data->events[i].data.fd, ev_loop->thread_name);
            channel_event_activate(ev_loop, ep_dis_data->events[i].data.fd, EVENT_READ);
        }

        if (ep_dis_data->events[i].events & EPOLLOUT) {
            x_msgx("get message channel fd:%d, fro write, %s", ep_dis_data->events[i].data.fd, ev_loop->thread_name);
            channel_event_activate(ev_loop, ep_dis_data->events[i].data.fd, EVENT_WRITE);
        }
    }

    return 0;
}

void epoll_clear(struct event_loop *ev_loop) {
    epoll_dispatcher_data *ep_dis_data = (epoll_dispatcher_data *) ev_loop->event_dispatcher_data;

    free(ep_dis_data->events);
    close(ep_dis_data->efd);
    free(ep_dis_data);
    ev_loop->event_dispatcher_data = NULL;

    return;
}