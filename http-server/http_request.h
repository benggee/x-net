#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#define MAX_QUERY_PARAMS 32
#define MAX_COOKIES 32
#define MAX_PATH_LENGTH 256

struct request_header {
    char *key;
    char *value;
};

// Query parameter key-value pair
struct query_param {
    char *key;
    char *value;
};

// Cookie key-value pair
struct cookie {
    char *name;
    char *value;
};

enum http_method {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_HEAD,
    HTTP_OPTIONS,
    HTTP_PATCH,
    HTTP_UNKNOWN
};

enum http_request_state {
    REQUEST_STATUS,  // wait parse state
    REQUEST_HEADERS, // wait parse headers
    REQUEST_BODY,    // wait parse body
    REQUEST_DONE     // parse done
};

struct http_request {
    char *version;
    char *method_str;
    enum http_method method;
    char *url;                // Original URL
    char *path;               // URL path (without query string)
    struct query_param *query_params;
    int query_params_count;
    enum http_request_state current_state;
    struct request_header *request_headers;  // header array
    int request_headers_number;              // header number
    struct cookie *cookies;
    int cookies_count;
    char *body;
    int body_length;
};

struct http_request *http_request_new();

void http_request_clear(struct http_request *http_req);

void http_request_reset(struct http_request *http_req);

void http_request_add_header(struct http_request *http_req, char *key, char *value);

char *http_request_get_header(struct http_request *http_req, char *key);

enum http_request_state http_request_current_state(struct http_request *http_req);

int http_request_close_connection(struct http_request *http_req);

// Get HTTP method enum from string
enum http_method http_request_get_method(struct http_request *http_req);

// Get query parameter value by key
char *http_request_get_param(struct http_request *http_req, const char *key);

// Get cookie value by name
char *http_request_get_cookie(struct http_request *http_req, const char *name);

// Parse query string from URL
void http_request_parse_query_string(struct http_request *http_req);

// Parse cookies from Cookie header
void http_request_parse_cookies(struct http_request *http_req);

// Check if request is for specific path
int http_request_path_matches(struct http_request *http_req, const char *path);

#endif