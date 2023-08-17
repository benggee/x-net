#include "http_response.h"
#include "common.h"

#define INIT_RESPONSE_HEADER_SIZE 128

struct http_response *http_response_new() {
    struct http_response *http_reply = malloc(sizeof(struct http_response));
    http_reply->body = NULL;
    http_reply->status_code = Unknown;
    http_reply->status_message = NULL;
    http_reply->response_headers = malloc(sizeof(struct response_header) * INIT_RESPONSE_HEADER_SIZE);
    http_reply->response_headers_number = 0;
    http_reply->keep_connected = 0;
    return  http_reply;
}

void http_response_encode_buffer(struct http_response *http_reply, struct buffer *output) {
    char buf[32];
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", http_reply->status_code);
    buffer_append_string(output, buf);
    buffer_append_string(output, http_reply->status_message);
    buffer_append_string(output, "\r\n");

    if (http_reply->keep_connected) {
        buffer_append_string(output, "Connection: close\r\n");
    } else {
        snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", strlen(http_reply->body));
        buffer_append_string(output, buf);
        buffer_append_string(output, "Connection: Keep-Alive\r\n");
    }

    if (http_reply->response_headers != NULL && http_reply->response_headers_number >0) {
        for (int i = 0; i < http_reply->response_headers_number; i++) {
            buffer_append_string(output, http_reply->response_headers[i].key);
            buffer_append_string(output, ": ");
            buffer_append_string(output, http_reply->response_headers[i].value);
            buffer_append_string(output, "\r\n");
        }
    }

    buffer_append_string(output, "\r\n");
    buffer_append_string(output, http_reply->body);
}