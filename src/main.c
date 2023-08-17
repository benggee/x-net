#include "event_loop.h"
#include "common.h"
#include "tcp_server.h"
#include "acceptor.h"
#include "buffer.h"

char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

//连接建立之后的callback
int onConnectionCompleted(struct tcp_connection *tcpConnection) {
    printf("Connection completed\n");
    return 0;
}

//数据读到buffer之后的callback
int onMessage(struct buffer *input, struct tcp_connection *tcpConnection) {
    printf("get message from tcp connection %s\n", tcpConnection->name);
    printf("%s", input->data);

    struct buffer *output = buffer_new();
    int size = buffer_readable_size(input);
    for (int i = 0; i < size; i++) {
        buffer_append_char(output, rot13_char(buffer_read_char(input)));
    }
    tcp_connection_send_buffer(tcpConnection, output);
    return 0;
}

//数据通过buffer写完之后的callback
int onWriteCompleted(struct tcp_connection *tcpConnection) {
    printf("write completed\n");
    return 0;
}

//连接关闭之后的callback
int onConnectionClosed(struct tcp_connection *tcpConnection) {
    printf("connection closed\n");
    return 0;
}

int main(int argc, char *argv[]) {

    struct event_loop *ev_loop = event_loop_init();

    struct acceptor *acceptor = acceptor_init(SERV_PORT);

    struct TCPServer *tcp_server = tcp_server_init(ev_loop, acceptor, onConnectionCompleted, onMessage,
                                                   onWriteCompleted, onConnectionClosed, 4);

    tcp_server_start(tcp_server);

    event_loop_run(ev_loop);
}