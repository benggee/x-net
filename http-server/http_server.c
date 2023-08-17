#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "http_server.h"
#include "log.h"
#include "buffer.h"
#include "utils.h"


int http_on_connection_completed(struct tcp_connection *tcp_conn) {
    x_msgx("connection completed");

    struct http_request *http_req = http_request_new();
    tcp_conn->request = http_req;
    return 0;
}

int process_status_line(char *start, char *end, struct http_request *http_req) {
    int size = end - start;

    // get method
    char *space = memmemx(start, end - start, " ", 1);
    assert(space != NULL);

    int method_size = space - start;
    http_req->method = malloc(method_size + 1);
    strncpy(http_req->method, start, method_size);
    http_req->method[method_size] = '\0';

    assert(space + 1 < end);
    // get url
    start = space + 1;
    assert(start < end);
    space = memmemx(start, end - start, " ", 1);
    assert(space != NULL);

    int url_size = space - start;
    http_req->url = malloc(url_size + 1);
    strncpy(http_req->url, start, space - start);
    http_req->url[url_size + 1] = '\0';

    // get version
    start = space + 1;
    http_req->version = malloc(end - start +1);
    strncpy(http_req->version, start, end - start);
    http_req->version[end - start + 1] = '\0';
    assert(space != NULL);
    return size;
}

int parse_http_request(struct buffer *input, struct http_request *http_req) {
    int ok = 1;
    while (http_req->current_state != REQUEST_DONE) {
        if (http_req->current_state == REQUEST_STATUS) {
            char *crlf = buffer_find_CRLF(input);
            if (crlf) {
                int request_line_size = process_status_line(input->data + input->read_index, crlf, http_req);
                if (request_line_size) {
                    input->read_index += request_line_size;
                    input->read_index += 2;
                    http_req->current_state = REQUEST_HEADERS;
                }
            }
        } else if (http_req->current_state == REQUEST_HEADERS) {
            char *crlf = buffer_find_CRLF(input);
            if (crlf) {
                char *start = input->data + input->read_index;
                int request_line_size = crlf - start;
                char *colon = memmemx(start, request_line_size, ": ", 2);
                if (colon != NULL) {
                    char *key = malloc(colon - start + 1);
                    strncpy(key, start, colon - start);
                    key[colon - start] = '\0';

                    char *value = malloc(crlf - colon - 2 +1);
                    strncpy(value, colon + 2, crlf - colon -2);
                    value[crlf - colon -2] = '\0';

                    http_request_add_header(http_req, key, value);

                    input->read_index += request_line_size;
                    input->read_index += 2;
                } else {
                    input->read_index += 2;
                    http_req->current_state = REQUEST_DONE;
                }
            }
        }
    }

    return ok;
}

int http_on_message(struct buffer *input, struct tcp_connection *tcp_conn) {
    x_debugx("get message from tcp connection %s", tcp_conn->name);

    struct http_request *http_req = (struct http_request *) tcp_conn->request;
    struct http_server *http_serv = (struct http_server *) tcp_conn->data;

    if (parse_http_request(input, http_req) == 0) {
        char *error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        tcp_connection_send_data(tcp_conn, error_response, sizeof(error_response));
        tcp_connection_shutdown(tcp_conn);
    }
    if (http_request_current_state(http_req) == REQUEST_DONE) {
        struct http_response *http_reply = http_response_new();

        if (http_serv->req_callback != NULL) {
            http_serv->req_callback(http_req, http_reply);
        }

        struct buffer *buf = buffer_new();
        http_response_encode_buffer(http_reply, buf);
        tcp_connection_send_buffer(tcp_conn, buf);

        if (http_request_close_connection(http_req)) {
            tcp_connection_shutdown(tcp_conn);
        }
        http_request_reset(http_req);
    }
}

int http_on_write_completed(struct tcp_connection *tcp_conn) {
    x_msgx("write completed");
    return 0;
}

int http_on_connection_closed(struct tcp_connection *tcp_conn) {
    x_msgx("connection closed");
    if (tcp_conn->request != NULL) {
        http_request_clear(tcp_conn->request);
        tcp_conn->request = NULL;
    }
    return 0;
}

struct http_server *http_server_new(struct event_loop *ev_loop, int port, request_callback req_callback, int thread_num) {
    struct http_server *http_serv = malloc(sizeof(struct http_server));
    http_serv->req_callback = req_callback;

    struct acceptor *acceptor = acceptor_init(SERV_PORT);

    http_serv->tcp_serv = tcp_server_init(ev_loop, acceptor, http_on_connection_completed, http_on_message,
                                            http_on_write_completed,
                                            http_on_connection_closed, thread_num);

    http_serv->tcp_serv->data = http_serv;

    return http_serv;
}

void http_server_start(struct http_server *http_serv) {
    tcp_server_start(http_serv->tcp_serv);
}