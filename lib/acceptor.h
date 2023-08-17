#ifndef ACCEPTOR_H
#define ACCEPTOR_H

struct acceptor {
    int listen_port;
    int listen_fd;
};

struct acceptor *acceptor_init(int port);

#endif