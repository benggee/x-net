#include "buffer.h"

#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

struct response_header {
    char *key;
    char *value;
};

enum http_status_code {
    Unknown,
    OK = 200,
    MovedPermanently = 301,
    BadRequest = 400,
    NotFound = 404,
};

struct http_response {
    enum http_status_code status_code;
    char *status_message;
    char *content_type;
    char *body;
    struct response_header *response_headers;
    int response_headers_number;
    int keep_connected;
};

struct http_response *http_response_new();

void http_response_encode_buffer(struct http_response *http_req, struct buffer *buffer);

#endif