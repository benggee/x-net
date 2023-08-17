#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include "common.h"
#include "tcp_server.h"
#include "http_request.h"
#include "http_response.h"

typedef int (*request_callback)(struct http_request *http_req, struct http_response *http_reply);

struct http_server {
    struct TCPServer *tcp_serv;
    request_callback req_callback;
};

struct http_server *http_server_new(struct event_loop *ev_loop, int port, request_callback req_callback, int thread_num);

void http_server_start(struct http_server *http_serv);

int parse_http_request(struct buffer *input, struct http_request *http_req);

#endif