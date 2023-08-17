#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "event_loop.h"
#include "channel.h"
#include "buffer.h"
#include "tcp_server.h"

struct tcp_connection {
    struct event_loop *ev_loop;
    struct channel *channel;
    char *name;
    struct buffer *input_buffer;
    struct buffer *output_buffer;

    connection_completed_callback conn_completed_callback;
    message_callback msg_callback;
    write_completed_callback w_completed_callback;
    connection_closed_callback conn_closed_callback;

    void *data;
    void *request;
    void *response;
};

struct tcp_connection *tcp_connection_new(int fd, struct event_loop *ev_loop, connection_completed_callback conn_completed_callback,
                                         message_callback msg_callback,
                                         write_completed_callback w_completed_callback,
                                         connection_closed_callback conn_closed_callback);

int tcp_connection_send_data(struct tcp_connection *tcp_conn, void *data, int size);

int tcp_connection_send_buffer(struct tcp_connection *tcp_conn, struct buffer *buffer);

int tcp_connection_shutdown(struct tcp_connection *tcp_conn);

int handle_read(void *data);

int handle_write(void *data);

#endif