#include "common.h"
#include "http_server.h"

#define INIT_REQUEST_HEADER_SIZE 128

const char *HTTP10 = "HTTP/1.0";
const char *HTTP11 = "HTTP/1.1";
const char *KEEP_ALIVE = "Keep-Alive";
const char *CLOSE = "close";

struct http_request *http_request_new() {
    struct http_request *http_req = malloc(sizeof(struct http_request));
    http_req->method = NULL;
    http_req->current_state = REQUEST_STATUS;
    http_req->version = NULL;
    http_req->url = NULL;
    http_req->request_headers = malloc(sizeof(struct http_request) * INIT_REQUEST_HEADER_SIZE);
    http_req->request_headers_number = 0;
    return http_req;
}

void http_request_clear(struct http_request *http_req) {
    if (http_req->request_headers != NULL) {
        for (int i = 0; i < http_req->request_headers_number; i++) {
            free(http_req->request_headers[i].key);
            free(http_req->request_headers[i].value);
        }
        free(http_req->request_headers);
    }
    free(http_req);
}

void http_request_reset(struct http_request *http_req) {
    http_req->method = NULL;
    http_req->current_state = REQUEST_STATUS;
    http_req->version = NULL;
    http_req->url = NULL;
    http_req->request_headers_number = 0;
}

void http_request_add_header(struct http_request *http_req, char *key, char *value) {
    http_req->request_headers[http_req->request_headers_number].key = key;
    http_req->request_headers[http_req->request_headers_number].value = value;
    http_req->request_headers_number++;
}

char *http_request_get_header(struct http_request *http_req, char *key) {
    if (http_req->request_headers != NULL) {
        for (int i = 0; i < http_req->request_headers_number; i++) {
            if (strncmp(http_req->request_headers[i].key, key, strlen(key)) == 0) {
                return http_req->request_headers[i].value;
            }
        }
    }
    return NULL;
}

enum http_request_state http_request_current_state(struct http_request *http_req) {
    return http_req->current_state;
}

int http_request_close_connection(struct http_request *http_req) {
    char *connection = http_request_get_header(http_req, "Connection");

    if (connection != NULL && strncmp(connection, CLOSE, strlen(CLOSE)) == 0) {
        return 1;
    }

    if (http_req->version != NULL
        && strncmp(http_req->version, HTTP10, strlen(HTTP10)) == 0
        && strncmp(connection, KEEP_ALIVE, strlen(KEEP_ALIVE)) == 1) {
        return 1;
    }

    return 0;
}