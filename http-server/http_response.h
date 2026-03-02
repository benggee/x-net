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
    Created = 201,
    MovedPermanently = 301,
    BadRequest = 400,
    NotFound = 404,
    MethodNotAllowed = 405,
    InternalServerError = 500,
    ServiceUnavailable = 503,
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

// Add a header to the response
void http_response_add_header(struct http_response *resp, char *key, char *value);

// Set response status
void http_response_set_status(struct http_response *resp, enum http_status_code code, const char *message);

// Set response body
void http_response_set_body(struct http_response *resp, char *body);

// Send JSON response
void http_response_json(struct http_response *resp, const char *json_data);

// Send HTML response
void http_response_html(struct http_response *resp, const char *html_data);

// Send file download response
void http_response_file(struct http_response *resp, const char *filename, const char *content);

// Set cookie in response
void http_response_set_cookie(struct http_response *resp, const char *name, const char *value,
                             int max_age, const char *path, const char *domain, int secure, int httponly);

// Free response resources
void http_response_free(struct http_response *resp);

#endif