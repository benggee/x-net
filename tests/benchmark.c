#include "../lib/buffer.h"
#include "../http-server/http_request.h"
#include "../http-server/http_response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Simple timing macros
#define BENCHMARK_START() clock_t start = clock()
#define BENCHMARK_END(name) clock_t end = clock(); \
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC; \
    printf("%s: %.6f seconds\n", name, elapsed)

void benchmark_buffer_operations() {
    printf("\n=== Buffer Performance Benchmarks ===\n");

    const int ITERATIONS = 100000;
    const char *test_data = "This is a test string for buffer operations benchmarking.";

    // Benchmark buffer_append
    BENCHMARK_START();
    for (int i = 0; i < ITERATIONS; i++) {
        struct buffer *buf = buffer_new();
        buffer_append(buf, (void *)test_data, strlen(test_data));
        buffer_free(buf);
    }
    BENCHMARK_END("buffer_append (100k iterations)");

    // Benchmark buffer_append_string
    BENCHMARK_START();
    for (int i = 0; i < ITERATIONS; i++) {
        struct buffer *buf = buffer_new();
        buffer_append_string(buf, (char *)test_data);
        buffer_free(buf);
    }
    BENCHMARK_END("buffer_append_string (100k iterations)");

    // Benchmark buffer_append_char
    BENCHMARK_START();
    for (int i = 0; i < ITERATIONS; i++) {
        struct buffer *buf = buffer_new();
        for (int j = 0; j < 100; j++) {
            buffer_append_char(buf, 'A');
        }
        buffer_free(buf);
    }
    BENCHMARK_END("buffer_append_char 100x (100k iterations)");

    // Benchmark buffer_read
    struct buffer *read_buf = buffer_new();
    buffer_append_string(read_buf, (char *)test_data);
    char data[256];

    BENCHMARK_START();
    for (int i = 0; i < ITERATIONS; i++) {
        int read = buffer_read(read_buf, data, sizeof(data));
        // Reset buffer
        read_buf->read_index = 0;
    }
    BENCHMARK_END("buffer_read (100k iterations)");

    buffer_free(read_buf);

    // Benchmark large buffer operations
    const int LARGE_SIZE = 1024 * 1024; // 1MB
    char *large_data = malloc(LARGE_SIZE);
    memset(large_data, 'A', LARGE_SIZE);

    BENCHMARK_START();
    struct buffer *large_buf = buffer_new();
    buffer_append(large_buf, large_data, LARGE_SIZE);
    buffer_free(large_buf);
    BENCHMARK_END("buffer_append 1MB");

    free(large_data);
}

void benchmark_http_operations() {
    printf("\n=== HTTP Performance Benchmarks ===\n");

    const int ITERATIONS = 10000;

    // Benchmark HTTP request creation
    BENCHMARK_START();
    for (int i = 0; i < ITERATIONS; i++) {
        struct http_request *req = http_request_new();
        http_request_clear(req);
    }
    BENCHMARK_END("http_request_new/clear (10k iterations)");

    // Benchmark HTTP response creation
    BENCHMARK_START();
    for (int i = 0; i < ITERATIONS; i++) {
        struct http_response *resp = http_response_new();
        http_response_free(resp);
    }
    BENCHMARK_END("http_response_new/free (10k iterations)");

    // Benchmark HTTP request parsing simulation
    struct http_request *req = http_request_new();
    req->url = strdup("/api/users?id=123&name=test");

    BENCHMARK_START();
    for (int i = 0; i < ITERATIONS; i++) {
        http_request_parse_query_string(req);
        // Reset for next iteration
        req->query_params_count = 0;
    }
    BENCHMARK_END("http_request_parse_query_string (10k iterations)");

    http_request_clear(req);

    // Benchmark HTTP response encoding
    struct http_response *resp = http_response_new();
    http_response_set_status(resp, OK, "OK");
    http_response_set_body(resp, strdup("{\"message\":\"Hello, World!\"}"));
    http_response_add_header(resp, strdup("Content-Type"), strdup("application/json"));

    BENCHMARK_START();
    for (int i = 0; i < ITERATIONS; i++) {
        struct buffer *buf = buffer_new();
        http_response_encode_buffer(resp, buf);
        buffer_free(buf);
    }
    BENCHMARK_END("http_response_encode_buffer (10k iterations)");

    http_response_free(resp);
}

void benchmark_http_headers() {
    printf("\n=== HTTP Header Performance Benchmarks ===\n");

    const int ITERATIONS = 10000;

    struct http_request *req = http_request_new();

    // Add multiple headers
    const char *header_names[] = {"Content-Type", "User-Agent", "Accept", "Authorization",
                                   "Host", "Connection", "Cache-Control", "Accept-Encoding"};
    const char *header_values[] = {"application/json", "TestClient/1.0", "*/*", "Bearer token123",
                                    "localhost:8080", "keep-alive", "no-cache", "gzip, deflate"};

    BENCHMARK_START();
    for (int i = 0; i < ITERATIONS; i++) {
        for (int j = 0; j < 8; j++) {
            http_request_add_header(req, strdup(header_names[j]), strdup(header_values[j]));
        }
        // Reset for next iteration
        req->request_headers_number = 0;
    }
    BENCHMARK_END("http_request_add_header 8 headers (10k iterations)");

    http_request_clear(req);
}

void benchmark_mixed_operations() {
    printf("\n=== Mixed Operations Benchmark ===\n");

    const int ITERATIONS = 10000;

    BENCHMARK_START();
    for (int i = 0; i < ITERATIONS; i++) {
        // Create request
        struct http_request *req = http_request_new();
        req->url = strdup("/api/test?key=value&data=test");
        http_request_parse_query_string(req);

        // Create response
        struct http_response *resp = http_response_new();
        http_response_set_status(resp, OK, "OK");
        http_response_set_body(resp, strdup("{\"status\":\"ok\"}"));
        http_response_add_header(resp, strdup("Content-Type"), strdup("application/json"));

        // Encode response
        struct buffer *buf = buffer_new();
        http_response_encode_buffer(resp, buf);

        // Clean up
        buffer_free(buf);
        http_request_clear(req);
        http_response_free(resp);
    }
    BENCHMARK_END("Complete HTTP request/response cycle (10k iterations)");
}

void print_system_info() {
    printf("=== System Information ===\n");
    printf("CLOCKS_PER_SEC: %ld\n", CLOCKS_PER_SEC);
    printf("Buffer INIT_BUFFER_SIZE: %d\n", INIT_BUFFER_SIZE);
    printf("sizeof(struct buffer): %zu\n", sizeof(struct buffer));
    printf("sizeof(struct http_request): %zu\n", sizeof(struct http_request));
    printf("sizeof(struct http_response): %zu\n", sizeof(struct http_response));
}

int main() {
    printf("========================================\n");
    printf("    X-Net Performance Benchmark Suite   \n");
    printf("========================================\n");

    print_system_info();

    benchmark_buffer_operations();
    benchmark_http_operations();
    benchmark_http_headers();
    benchmark_mixed_operations();

    printf("\n=== All Benchmarks Completed ===\n");
    return 0;
}
