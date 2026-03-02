#include "http_response.h"
#include "common.h"

#define INIT_RESPONSE_HEADER_SIZE 128

struct http_response *http_response_new() {
    struct http_response *http_reply = malloc(sizeof(struct http_response));
    if (!http_reply) {
        return NULL;
    }

    http_reply->body = NULL;
    http_reply->status_code = Unknown;
    http_reply->status_message = NULL;
    http_reply->response_headers = malloc(sizeof(struct response_header) * INIT_RESPONSE_HEADER_SIZE);
    if (!http_reply->response_headers) {
        free(http_reply);
        return NULL;
    }
    http_reply->response_headers_number = 0;
    http_reply->keep_connected = 0;
    return http_reply;
}

void http_response_encode_buffer(struct http_response *http_reply, struct buffer *output) {
    if (!http_reply || !output) {
        return;
    }

    char buf[32];
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", http_reply->status_code);
    buffer_append_string(output, buf);
    if (http_reply->status_message) {
        buffer_append_string(output, http_reply->status_message);
    }
    buffer_append_string(output, "\r\n");

    if (http_reply->keep_connected) {
        buffer_append_string(output, "Connection: close\r\n");
    } else {
        if (http_reply->body) {
            snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", strlen(http_reply->body));
            buffer_append_string(output, buf);
        }
        buffer_append_string(output, "Connection: Keep-Alive\r\n");
    }

    if (http_reply->response_headers != NULL && http_reply->response_headers_number > 0) {
        for (int i = 0; i < http_reply->response_headers_number; i++) {
            buffer_append_string(output, http_reply->response_headers[i].key);
            buffer_append_string(output, ": ");
            buffer_append_string(output, http_reply->response_headers[i].value);
            buffer_append_string(output, "\r\n");
        }
    }

    buffer_append_string(output, "\r\n");
    if (http_reply->body) {
        buffer_append_string(output, http_reply->body);
    }
}

void http_response_add_header(struct http_response *resp, char *key, char *value) {
    if (!resp || !key || !value) {
        return;
    }

    // Check if we have space for more headers
    if (resp->response_headers_number >= INIT_RESPONSE_HEADER_SIZE) {
        return;
    }

    resp->response_headers[resp->response_headers_number].key = key;
    resp->response_headers[resp->response_headers_number].value = value;
    resp->response_headers_number++;
}

void http_response_set_status(struct http_response *resp, enum http_status_code code, const char *message) {
    if (!resp) {
        return;
    }

    resp->status_code = code;
    if (message) {
        // Free old status message if exists
        if (resp->status_message) {
            free(resp->status_message);
        }
        resp->status_message = strdup(message);
    } else {
        // Set default status messages
        switch (code) {
            case OK:
                resp->status_message = strdup("OK");
                break;
            case Created:
                resp->status_message = strdup("Created");
                break;
            case MovedPermanently:
                resp->status_message = strdup("Moved Permanently");
                break;
            case BadRequest:
                resp->status_message = strdup("Bad Request");
                break;
            case NotFound:
                resp->status_message = strdup("Not Found");
                break;
            case MethodNotAllowed:
                resp->status_message = strdup("Method Not Allowed");
                break;
            case InternalServerError:
                resp->status_message = strdup("Internal Server Error");
                break;
            case ServiceUnavailable:
                resp->status_message = strdup("Service Unavailable");
                break;
            default:
                resp->status_message = strdup("Unknown");
                break;
        }
    }
}

void http_response_set_body(struct http_response *resp, char *body) {
    if (!resp) {
        return;
    }

    // Free old body if exists
    if (resp->body) {
        free(resp->body);
    }
    resp->body = body;
}

void http_response_json(struct http_response *resp, const char *json_data) {
    if (!resp || !json_data) {
        return;
    }

    http_response_set_status(resp, OK, NULL);
    http_response_set_body(resp, strdup(json_data));
    http_response_add_header(resp, strdup("Content-Type"), strdup("application/json"));
}

void http_response_html(struct http_response *resp, const char *html_data) {
    if (!resp || !html_data) {
        return;
    }

    http_response_set_status(resp, OK, NULL);
    http_response_set_body(resp, strdup(html_data));
    http_response_add_header(resp, strdup("Content-Type"), strdup("text/html"));
}

void http_response_file(struct http_response *resp, const char *filename, const char *content) {
    if (!resp || !filename || !content) {
        return;
    }

    http_response_set_status(resp, OK, NULL);
    http_response_set_body(resp, strdup(content));
    http_response_add_header(resp, strdup("Content-Type"), strdup("application/octet-stream"));

    char header[256];
    snprintf(header, sizeof(header), "attachment; filename=\"%s\"", filename);
    http_response_add_header(resp, strdup("Content-Disposition"), strdup(header));
}

void http_response_set_cookie(struct http_response *resp, const char *name, const char *value,
                             int max_age, const char *path, const char *domain, int secure, int httponly) {
    if (!resp || !name || !value) {
        return;
    }

    char cookie[512];
    int len = snprintf(cookie, sizeof(cookie), "%s=%s", name, value);

    if (max_age > 0) {
        len += snprintf(cookie + len, sizeof(cookie) - len, "; Max-Age=%d", max_age);
    }

    if (path) {
        len += snprintf(cookie + len, sizeof(cookie) - len, "; Path=%s", path);
    }

    if (domain) {
        len += snprintf(cookie + len, sizeof(cookie) - len, "; Domain=%s", domain);
    }

    if (secure) {
        len += snprintf(cookie + len, sizeof(cookie) - len, "; Secure");
    }

    if (httponly) {
        len += snprintf(cookie + len, sizeof(cookie) - len, "; HttpOnly");
    }

    http_response_add_header(resp, strdup("Set-Cookie"), strdup(cookie));
}

void http_response_free(struct http_response *resp) {
    if (!resp) {
        return;
    }

    if (resp->status_message) {
        free(resp->status_message);
    }

    if (resp->body) {
        free(resp->body);
    }

    if (resp->response_headers != NULL) {
        for (int i = 0; i < resp->response_headers_number; i++) {
            if (resp->response_headers[i].key) {
                free(resp->response_headers[i].key);
            }
            if (resp->response_headers[i].value) {
                free(resp->response_headers[i].value);
            }
        }
        free(resp->response_headers);
    }

    if (resp->content_type) {
        free(resp->content_type);
    }

    free(resp);
}