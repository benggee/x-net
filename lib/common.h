#ifndef X_NET_H
#define X_NET_H

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/select.h>
#include <poll.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <string.h>

#ifdef EPOLL_ENABLE
#include <sys/epoll.h>
#endif

#define SERV_PORT 3000

void make_nonblocking(int fd);

#endif