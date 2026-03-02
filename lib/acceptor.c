#include <assert.h>
#include "acceptor.h"
#include "common.h"
#include "log.h"

struct acceptor *acceptor_init(int port) {
    struct acceptor *acc = malloc(sizeof(struct acceptor));
    if (!acc) {
        perror("malloc acceptor failed");
        return NULL;
    }

    acc->listen_port = port;
    acc->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (acc->listen_fd < 0) {
        perror("socket failed");
        free(acc);
        return NULL;
    }

    make_nonblocking(acc->listen_fd);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(acc->listen_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(acc->listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt failed");
        close(acc->listen_fd);
        free(acc);
        return NULL;
    }

    if (bind(acc->listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind error");
        close(acc->listen_fd);
        free(acc);
        return NULL;
    }

    if (listen(acc->listen_fd, 1024) < 0) {
        perror("listen error");
        close(acc->listen_fd);
        free(acc);
        return NULL;
    }

    x_msgx("server start at: %d", acc->listen_port);

    return acc;
}