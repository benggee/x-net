#include "tcp_connection.h"
#include "utils.h"
#include "common.h"
#include "log.h"

struct tcp_connection *tcp_connection_new(int fd, struct event_loop *ev_loop, connection_completed_callback conn_completed_callback,
                                         message_callback msg_callback,
                                         write_completed_callback w_completed_callback,
                                         connection_closed_callback conn_closed_callback) {
    struct tcp_connection *tcp_conn = malloc(sizeof(struct tcp_connection));
    tcp_conn->w_completed_callback = w_completed_callback;
    tcp_conn->msg_callback = msg_callback;
    tcp_conn->conn_completed_callback = conn_completed_callback;
    tcp_conn->conn_closed_callback = conn_closed_callback;
    tcp_conn->ev_loop = ev_loop;
    tcp_conn->input_buffer = buffer_new();
    tcp_conn->output_buffer = buffer_new();

    char *buf = malloc(16);
    sprintf(buf, "connection-%d\0", fd);
    tcp_conn->name = buf;

    struct channel *chan = channel_new(fd, EVENT_READ, handle_read, handle_write, tcp_conn);
    tcp_conn->channel = chan;

    if (tcp_conn->conn_completed_callback != NULL) {
        tcp_conn->conn_completed_callback(tcp_conn);
    }

    event_loop_add_channel_event(tcp_conn->ev_loop, fd, tcp_conn->channel);

    return tcp_conn;

}

int tcp_connection_send_data(struct tcp_connection *tcp_conn, void *data, int size) {
    size_t nwrited = 0;
    size_t nleft = size;
    int fault = 0;

    struct channel *chan = tcp_conn->channel;
    struct buffer *output_buffer = tcp_conn->output_buffer;

    if (!channel_write_event_is_enabled(chan) && buffer_readable_size(output_buffer) == 0) {
        nwrited = write(chan->fd, data, size);
        if (nwrited >= 0) {
            nleft = nleft - nwrited;
        } else {
            nwrited = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    fault = 1;
                }
            }
        }
    }

    if (!fault && nleft > 0) {
        buffer_append(output_buffer, data + nwrited, nleft);
        if (!channel_write_event_is_enabled(chan)) {
            channel_write_event_enable(chan);
        }
    }
    return nwrited;
}

int tcp_connection_send_buffer(struct tcp_connection *tcp_conn, struct buffer *buffer) {
    int size = buffer_readable_size(buffer);
    int result = tcp_connection_send_data(tcp_conn, buffer->data + buffer->read_index, size);
    buffer->read_index += size;
    return result;
}

int tcp_connection_shutdown(struct tcp_connection *tcp_conn) {
    if (shutdown(tcp_conn->channel->fd, SHUT_WR) < 0) {
        printf("tcp_connection_shutdown failed, socket == %d", tcp_conn->channel->fd);
    }
}

int handle_read(void *data) {
    struct tcp_connection *tcp_conn = (struct tcp_connection *) data;
    struct buffer *input_buffer = tcp_conn->input_buffer;
    struct channel *chan = tcp_conn->channel;

    if (buffer_socket_read(input_buffer, chan->fd) > 0) {
        x_msgx("read message, (%s)", input_buffer->data);
        if (tcp_conn->msg_callback != NULL) {
            tcp_conn->msg_callback(input_buffer, tcp_conn);
        }
    }
}

int handle_write(void *data) {
    struct tcp_connection *tcp_conn = (struct tcp_connection *) data;
    struct event_loop *ev_loop = tcp_conn->ev_loop;

    assert_in_same_thread(ev_loop);

    struct buffer *output_buffer = tcp_conn->output_buffer;
    struct channel *chan = tcp_conn->channel;

    ssize_t nwrited = write(chan->fd, output_buffer->data + output_buffer->read_index, buffer_readable_size(output_buffer));

    if (nwrited > 0) {
        output_buffer->read_index += nwrited;
        if (buffer_readable_size(output_buffer) == 0) {
            channel_write_event_disable(chan);
        }

        if (tcp_conn->w_completed_callback != NULL) {
            tcp_conn->w_completed_callback(tcp_conn);
        }
    }
}