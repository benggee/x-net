#include "common.h"
#include "http_server.h"

#define INIT_REQUEST_HEADER_SIZE 128

const char *HTTP10 = "HTTP/1.0";
const char *HTTP11 = "HTTP/1.1";
const char *KEEP_ALIVE = "Keep-Alive";
const char *CLOSE = "close";

struct http_request *http_request_new() {
    struct http_request *http_req = malloc(sizeof(struct http_request));
    if (!http_req) {
        return NULL;
    }

    http_req->method_str = NULL;
    http_req->method = HTTP_UNKNOWN;
    http_req->current_state = REQUEST_STATUS;
    http_req->version = NULL;
    http_req->url = NULL;
    http_req->path = NULL;
    http_req->request_headers = malloc(sizeof(struct request_header) * INIT_REQUEST_HEADER_SIZE);
    if (!http_req->request_headers) {
        free(http_req);
        return NULL;
    }
    http_req->request_headers_number = 0;

    http_req->query_params = malloc(sizeof(struct query_param) * MAX_QUERY_PARAMS);
    if (!http_req->query_params) {
        free(http_req->request_headers);
        free(http_req);
        return NULL;
    }
    http_req->query_params_count = 0;

    http_req->cookies = malloc(sizeof(struct cookie) * MAX_COOKIES);
    if (!http_req->cookies) {
        free(http_req->query_params);
        free(http_req->request_headers);
        free(http_req);
        return NULL;
    }
    http_req->cookies_count = 0;

    http_req->body = NULL;
    http_req->body_length = 0;

    return http_req;
}

void http_request_clear(struct http_request *http_req) {
    if (!http_req) {
        return;
    }

    if (http_req->method_str) {
        free(http_req->method_str);
    }

    if (http_req->version) {
        free(http_req->version);
    }

    if (http_req->url) {
        free(http_req->url);
    }

    if (http_req->path) {
        free(http_req->path);
    }

    if (http_req->request_headers != NULL) {
        for (int i = 0; i < http_req->request_headers_number; i++) {
            if (http_req->request_headers[i].key) {
                free(http_req->request_headers[i].key);
            }
            if (http_req->request_headers[i].value) {
                free(http_req->request_headers[i].value);
            }
        }
        free(http_req->request_headers);
    }

    if (http_req->query_params != NULL) {
        for (int i = 0; i < http_req->query_params_count; i++) {
            if (http_req->query_params[i].key) {
                free(http_req->query_params[i].key);
            }
            if (http_req->query_params[i].value) {
                free(http_req->query_params[i].value);
            }
        }
        free(http_req->query_params);
    }

    if (http_req->cookies != NULL) {
        for (int i = 0; i < http_req->cookies_count; i++) {
            if (http_req->cookies[i].name) {
                free(http_req->cookies[i].name);
            }
            if (http_req->cookies[i].value) {
                free(http_req->cookies[i].value);
            }
        }
        free(http_req->cookies);
    }

    if (http_req->body) {
        free(http_req->body);
    }

    free(http_req);
}

void http_request_reset(struct http_request *http_req) {
    if (!http_req) {
        return;
    }

    // Free previously allocated memory
    if (http_req->method_str) {
        free(http_req->method_str);
        http_req->method_str = NULL;
    }

    if (http_req->url) {
        free(http_req->url);
        http_req->url = NULL;
    }

    if (http_req->path) {
        free(http_req->path);
        http_req->path = NULL;
    }

    if (http_req->version) {
        free(http_req->version);
        http_req->version = NULL;
    }

    // Free headers
    if (http_req->request_headers != NULL) {
        for (int i = 0; i < http_req->request_headers_number; i++) {
            if (http_req->request_headers[i].key) {
                free(http_req->request_headers[i].key);
            }
            if (http_req->request_headers[i].value) {
                free(http_req->request_headers[i].value);
            }
        }
        http_req->request_headers_number = 0;
    }

    // Free query params
    if (http_req->query_params != NULL) {
        for (int i = 0; i < http_req->query_params_count; i++) {
            if (http_req->query_params[i].key) {
                free(http_req->query_params[i].key);
            }
            if (http_req->query_params[i].value) {
                free(http_req->query_params[i].value);
            }
        }
        http_req->query_params_count = 0;
    }

    // Free cookies
    if (http_req->cookies != NULL) {
        for (int i = 0; i < http_req->cookies_count; i++) {
            if (http_req->cookies[i].name) {
                free(http_req->cookies[i].name);
            }
            if (http_req->cookies[i].value) {
                free(http_req->cookies[i].value);
            }
        }
        http_req->cookies_count = 0;
    }

    // Free body
    if (http_req->body) {
        free(http_req->body);
        http_req->body = NULL;
    }
    http_req->body_length = 0;

    http_req->method = HTTP_UNKNOWN;
    http_req->current_state = REQUEST_STATUS;
}

void http_request_add_header(struct http_request *http_req, char *key, char *value) {
    if (!http_req || !key || !value) {
        return;
    }

    // Check if we have space for more headers
    if (http_req->request_headers_number >= INIT_REQUEST_HEADER_SIZE) {
        return;
    }

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

enum http_method http_request_get_method(struct http_request *http_req) {
    if (!http_req || !http_req->method_str) {
        return HTTP_UNKNOWN;
    }

    if (strcmp(http_req->method_str, "GET") == 0) {
        return HTTP_GET;
    } else if (strcmp(http_req->method_str, "POST") == 0) {
        return HTTP_POST;
    } else if (strcmp(http_req->method_str, "PUT") == 0) {
        return HTTP_PUT;
    } else if (strcmp(http_req->method_str, "DELETE") == 0) {
        return HTTP_DELETE;
    } else if (strcmp(http_req->method_str, "HEAD") == 0) {
        return HTTP_HEAD;
    } else if (strcmp(http_req->method_str, "OPTIONS") == 0) {
        return HTTP_OPTIONS;
    } else if (strcmp(http_req->method_str, "PATCH") == 0) {
        return HTTP_PATCH;
    }

    return HTTP_UNKNOWN;
}

char *http_request_get_param(struct http_request *http_req, const char *key) {
    if (!http_req || !key || !http_req->query_params) {
        return NULL;
    }

    for (int i = 0; i < http_req->query_params_count; i++) {
        if (strcmp(http_req->query_params[i].key, key) == 0) {
            return http_req->query_params[i].value;
        }
    }

    return NULL;
}

char *http_request_get_cookie(struct http_request *http_req, const char *name) {
    if (!http_req || !name || !http_req->cookies) {
        return NULL;
    }

    for (int i = 0; i < http_req->cookies_count; i++) {
        if (strcmp(http_req->cookies[i].name, name) == 0) {
            return http_req->cookies[i].value;
        }
    }

    return NULL;
}

void http_request_parse_query_string(struct http_request *http_req) {
    if (!http_req || !http_req->url) {
        return;
    }

    // Find query string start
    char *query_start = strchr(http_req->url, '?');
    if (!query_start) {
        // No query string, set path to full URL
        http_req->path = strdup(http_req->url);
        return;
    }

    // Extract path (before ?)
    size_t path_len = query_start - http_req->url;
    http_req->path = malloc(path_len + 1);
    if (!http_req->path) {
        return;
    }
    strncpy(http_req->path, http_req->url, path_len);
    http_req->path[path_len] = '\0';

    // Parse query parameters
    char *query = query_start + 1;
    char *saveptr = NULL;
    char *pair = strtok_r(query, "&", &saveptr);

    while (pair && http_req->query_params_count < MAX_QUERY_PARAMS) {
        char *equals = strchr(pair, '=');
        if (equals) {
            *equals = '\0';
            char *key = strdup(pair);
            char *value = strdup(equals + 1);

            if (key && value) {
                http_req->query_params[http_req->query_params_count].key = key;
                http_req->query_params[http_req->query_params_count].value = value;
                http_req->query_params_count++;
            } else {
                if (key) free(key);
                if (value) free(value);
            }
        }
        pair = strtok_r(NULL, "&", &saveptr);
    }
}

void http_request_parse_cookies(struct http_request *http_req) {
    if (!http_req) {
        return;
    }

    char *cookie_header = http_request_get_header(http_req, "Cookie");
    if (!cookie_header) {
        return;
    }

    char *cookies_str = strdup(cookie_header);
    if (!cookies_str) {
        return;
    }

    char *saveptr = NULL;
    char *pair = strtok_r(cookies_str, ";", &saveptr);

    while (pair && http_req->cookies_count < MAX_COOKIES) {
        // Trim leading whitespace
        while (*pair == ' ') {
            pair++;
        }

        char *equals = strchr(pair, '=');
        if (equals) {
            *equals = '\0';
            char *name = strdup(pair);
            char *value = strdup(equals + 1);

            if (name && value) {
                http_req->cookies[http_req->cookies_count].name = name;
                http_req->cookies[http_req->cookies_count].value = value;
                http_req->cookies_count++;
            } else {
                if (name) free(name);
                if (value) free(value);
            }
        }
        pair = strtok_r(NULL, ";", &saveptr);
    }

    free(cookies_str);
}

int http_request_path_matches(struct http_request *http_req, const char *path) {
    if (!http_req || !path || !http_req->path) {
        return 0;
    }

    return strcmp(http_req->path, path) == 0;
}