#include "../http-server/http_request.h"
#include "../http-server/http_response.h"
#include "../lib/buffer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_http_request_new() {
    struct http_request *req = http_request_new();
    assert(req != NULL);
    assert(req->method == HTTP_UNKNOWN);
    assert(req->current_state == REQUEST_STATUS);
    assert(req->request_headers != NULL);
    assert(req->query_params != NULL);
    assert(req->cookies != NULL);

    http_request_clear(req);
    printf("test_http_request_new: PASSED\n");
}

void test_http_request_add_header() {
    struct http_request *req = http_request_new();
    http_request_add_header(req, strdup("Content-Type"), strdup("application/json"));
    http_request_add_header(req, strdup("User-Agent"), strdup("TestClient"));

    assert(req->request_headers_number == 2);
    assert(strcmp(http_request_get_header(req, "Content-Type"), "application/json") == 0);

    http_request_clear(req);
    printf("test_http_request_add_header: PASSED\n");
}

void test_http_request_parse_query_string() {
    struct http_request *req = http_request_new();
    req->url = strdup("/test?key1=value1&key2=value2");

    http_request_parse_query_string(req);

    assert(req->path != NULL);
    assert(strcmp(req->path, "/test") == 0);
    assert(strcmp(http_request_get_param(req, "key1"), "value1") == 0);
    assert(strcmp(http_request_get_param(req, "key2"), "value2") == 0);

    http_request_clear(req);
    printf("test_http_request_parse_query_string: PASSED\n");
}

void test_http_request_parse_cookies() {
    struct http_request *req = http_request_new();
    http_request_add_header(req, strdup("Cookie"), strdup("session=abc123; user=john"));

    http_request_parse_cookies(req);

    assert(req->cookies_count == 2);
    assert(strcmp(http_request_get_cookie(req, "session"), "abc123") == 0);
    assert(strcmp(http_request_get_cookie(req, "user"), "john") == 0);

    http_request_clear(req);
    printf("test_http_request_parse_cookies: PASSED\n");
}

void test_http_request_path_matches() {
    struct http_request *req = http_request_new();
    req->path = strdup("/api/users");

    assert(http_request_path_matches(req, "/api/users") == 1);
    assert(http_request_path_matches(req, "/api/posts") == 0);

    http_request_clear(req);
    printf("test_http_request_path_matches: PASSED\n");
}

void test_http_request_get_method() {
    struct http_request *req = http_request_new();

    req->method_str = strdup("GET");
    assert(http_request_get_method(req) == HTTP_GET);

    free(req->method_str);
    req->method_str = strdup("POST");
    assert(http_request_get_method(req) == HTTP_POST);

    free(req->method_str);
    req->method_str = strdup("DELETE");
    assert(http_request_get_method(req) == HTTP_DELETE);

    http_request_clear(req);
    printf("test_http_request_get_method: PASSED\n");
}

void test_http_request_reset() {
    struct http_request *req = http_request_new();
    req->method_str = strdup("GET");
    req->url = strdup("/test");
    req->version = strdup("HTTP/1.1");
    http_request_add_header(req, strdup("Host"), strdup("localhost"));

    http_request_reset(req);

    assert(req->method_str == NULL);
    assert(req->url == NULL);
    assert(req->version == NULL);
    assert(req->request_headers_number == 0);
    assert(req->query_params_count == 0);
    assert(req->cookies_count == 0);

    http_request_clear(req);
    printf("test_http_request_reset: PASSED\n");
}

void test_http_response_new() {
    struct http_response *resp = http_response_new();
    assert(resp != NULL);
    assert(resp->status_code == Unknown);
    assert(resp->response_headers != NULL);

    http_response_free(resp);
    printf("test_http_response_new: PASSED\n");
}

void test_http_response_set_status() {
    struct http_response *resp = http_response_new();

    http_response_set_status(resp, OK, NULL);
    assert(resp->status_code == OK);
    assert(strcmp(resp->status_message, "OK") == 0);

    http_response_set_status(resp, NotFound, NULL);
    assert(resp->status_code == NotFound);
    assert(strcmp(resp->status_message, "Not Found") == 0);

    http_response_free(resp);
    printf("test_http_response_set_status: PASSED\n");
}

void test_http_response_set_body() {
    struct http_response *resp = http_response_new();

    http_response_set_body(resp, strdup("Test body content"));
    assert(resp->body != NULL);
    assert(strcmp(resp->body, "Test body content") == 0);

    http_response_free(resp);
    printf("test_http_response_set_body: PASSED\n");
}

void test_http_response_add_header() {
    struct http_response *resp = http_response_new();

    http_response_add_header(resp, strdup("Content-Type"), strdup("application/json"));
    assert(resp->response_headers_number == 1);
    assert(strcmp(resp->response_headers[0].key, "Content-Type") == 0);
    assert(strcmp(resp->response_headers[0].value, "application/json") == 0);

    http_response_free(resp);
    printf("test_http_response_add_header: PASSED\n");
}

void test_http_response_json() {
    struct http_response *resp = http_response_new();

    http_response_json(resp, "{\"message\":\"hello\"}");
    assert(resp->status_code == OK);
    assert(resp->body != NULL);
    assert(strcmp(resp->body, "{\"message\":\"hello\"}") == 0);

    // Check Content-Type header
    int found = 0;
    for (int i = 0; i < resp->response_headers_number; i++) {
        if (strcmp(resp->response_headers[i].key, "Content-Type") == 0 &&
            strcmp(resp->response_headers[i].value, "application/json") == 0) {
            found = 1;
            break;
        }
    }
    assert(found == 1);

    http_response_free(resp);
    printf("test_http_response_json: PASSED\n");
}

void test_http_response_html() {
    struct http_response *resp = http_response_new();

    http_response_html(resp, "<html><body>Hello</body></html>");
    assert(resp->status_code == OK);
    assert(resp->body != NULL);
    assert(strcmp(resp->body, "<html><body>Hello</body></html>") == 0);

    http_response_free(resp);
    printf("test_http_response_html: PASSED\n");
}

void test_http_response_set_cookie() {
    struct http_response *resp = http_response_new();

    http_response_set_cookie(resp, "session", "abc123", 3600, "/", "localhost", 0, 1);

    // Check if Set-Cookie header was added
    int found = 0;
    for (int i = 0; i < resp->response_headers_number; i++) {
        if (strcmp(resp->response_headers[i].key, "Set-Cookie") == 0) {
            found = 1;
            // Verify cookie format
            char *value = resp->response_headers[i].value;
            assert(strstr(value, "session=abc123") != NULL);
            assert(strstr(value, "Max-Age=3600") != NULL);
            assert(strstr(value, "Path=/") != NULL);
            assert(strstr(value, "HttpOnly") != NULL);
            break;
        }
    }
    assert(found == 1);

    http_response_free(resp);
    printf("test_http_response_set_cookie: PASSED\n");
}

void test_http_response_encode_buffer() {
    struct http_response *resp = http_response_new();
    struct buffer *buf = buffer_new();

    http_response_set_status(resp, OK, "OK");
    http_response_set_body(resp, strdup("Hello"));
    http_response_add_header(resp, strdup("Content-Type"), strdup("text/plain"));

    http_response_encode_buffer(resp, buf);

    assert(buffer_readable_size(buf) > 0);
    assert(strstr(buf->data, "HTTP/1.1 200 OK") != NULL);
    assert(strstr(buf->data, "Content-Type: text/plain") != NULL);
    assert(strstr(buf->data, "Hello") != NULL);

    buffer_free(buf);
    http_response_free(resp);
    printf("test_http_response_encode_buffer: PASSED\n");
}

int main() {
    printf("=== HTTP Unit Tests ===\n");

    test_http_request_new();
    test_http_request_add_header();
    test_http_request_parse_query_string();
    test_http_request_parse_cookies();
    test_http_request_path_matches();
    test_http_request_get_method();
    test_http_request_reset();
    test_http_response_new();
    test_http_response_set_status();
    test_http_response_set_body();
    test_http_response_add_header();
    test_http_response_json();
    test_http_response_html();
    test_http_response_set_cookie();
    test_http_response_encode_buffer();

    printf("\n=== All HTTP Tests Passed ===\n");
    return 0;
}
