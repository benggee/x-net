#include <assert.h>
#include "tcp_server.h"
#include "thread_pool.h"
#include "tcp_connection.h"
#include "acceptor.h"
#include "common.h"
#include "channel.h"
#include "log.h"

void make_nonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

struct TCPServer *tcp_server_init(struct event_loop *ev_loop, struct acceptor *acceptor,
                                  connection_completed_callback conn_completed_callback,
                                  message_callback msg_callback,
                                  write_completed_callback wt_completed_callback,
                                  connection_closed_callback conn_closed_callback,
                                  int thread_num) {
    struct TCPServer *tcp_server = malloc(sizeof(struct TCPServer));

    tcp_server->ev_loop = ev_loop;
    tcp_server->acceptor = acceptor;
    tcp_server->conn_completed_callback = conn_completed_callback;
    tcp_server->msg_callback = msg_callback;
    tcp_server->w_completed_callback = wt_completed_callback;
    tcp_server->conn_close_callback = conn_closed_callback;
    tcp_server->thread_num = thread_num;
    tcp_server->th_pool = thread_pool_new(ev_loop, thread_num);
    tcp_server->data = NULL;

    return tcp_server;
}

int handle_connection_established(void *data) {
    struct TCPServer *tcp_server = (struct TCPServer *) data;
    struct acceptor *acceptor = tcp_server->acceptor;
    int listen_fd = acceptor->listen_fd;

    x_msgx("acceptor fd=%d", acceptor->listen_fd);

    struct sockaddr_in client_addr;
    socklen_t cli_len = sizeof(client_addr);
    int conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &cli_len);
    make_nonblocking(conn_fd);

    x_msgx("new conn established, fd=%d", conn_fd);

    struct event_loop *ev_loop = thread_pool_get_loop(tcp_server->th_pool);

    struct tcp_connection *tcp_conn = tcp_connection_new(conn_fd, ev_loop,
                                                         tcp_server->conn_completed_callback,
                                                         tcp_server->msg_callback,
                                                         tcp_server->w_completed_callback,
                                                         tcp_server->conn_close_callback);
    if (tcp_server->data != NULL) {
        tcp_conn->data = tcp_server->data;
    }

    return 0;
}

void tcp_server_start(struct TCPServer *tcp_server) {
    struct acceptor *acceptor = tcp_server->acceptor;
    struct event_loop *ev_loop = tcp_server->ev_loop;

    thread_pool_start(tcp_server->th_pool);

    struct channel *chan = channel_new(acceptor->listen_fd, EVENT_READ, handle_connection_established, NULL, tcp_server);

    event_loop_add_channel_event(ev_loop, chan->fd, chan);

    return;
}

void tcp_server_set_data(struct TCPServer *tcp_server, void *data) {
    if (data != NULL) {
        tcp_server->data = data;
    }
}