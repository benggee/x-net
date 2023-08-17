#ifndef TCP_SERVER_H
#define TCP_SERVER_H

struct tcp_connection;
struct buffer;

typedef int (*connection_completed_callback)(struct tcp_connection *tcp_conn);

typedef int (*message_callback)(struct buffer *buf, struct tcp_connection *tcp_conn);

typedef int (*write_completed_callback) (struct tcp_connection *tcp_conn);

typedef int (*connection_closed_callback)(struct tcp_connection *tcp_conn);

#include "acceptor.h"
#include "event_loop.h"
#include "thread_pool.h"
#include "tcp_connection.h"
#include "buffer.h"

struct TCPServer {
    int port;
    struct event_loop *ev_loop;
    struct acceptor *acceptor;
    connection_completed_callback conn_completed_callback;
    message_callback msg_callback;
    write_completed_callback w_completed_callback;
    connection_closed_callback conn_close_callback;
    int thread_num;
    struct thread_pool *th_pool;
    void *data;
};

struct TCPServer *tcp_server_init(struct event_loop *ev_loop, struct acceptor *acceptor,
                                  connection_completed_callback conn_completed_callback,
                                  message_callback msg_callback,
                                  write_completed_callback wt_completed_callback,
                                  connection_closed_callback conn_closed_callback,
                                  int thread_num);

void tcp_server_start(struct TCPServer *tcp_server);

void tcp_server_set_data(struct TCPServer *tcp_server, void *data);

#endif