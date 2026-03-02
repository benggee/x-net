# X-Net

A high-performance, event-driven network framework written in C, designed for building scalable TCP and HTTP servers with ease.

## Features

### Core Features
- **Event-Driven Architecture**: Built on epoll for efficient I/O multiplexing
- **Thread Pool Support**: Multi-threaded connection handling for optimal performance
- **Circular Buffer**: Efficient data buffering with automatic expansion
- **Non-Blocking I/O**: All operations are non-blocking for maximum concurrency
- **Cross-Platform**: Designed for Linux with portable architecture

### HTTP Server Features
- **HTTP/1.0 and HTTP/1.1 Support**: Full compliance with HTTP standards
- **Multiple HTTP Methods**: GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH
- **Query String Parsing**: Automatic parsing and access to URL parameters
- **Cookie Management**: Built-in cookie parsing and generation
- **Static File Serving**: Efficient static file serving with MIME type detection
- **Keep-Alive Connections**: HTTP connection reuse for better performance
- **Flexible Response API**: Easy-to-use response building with helper functions

### Configuration
- **YAML Configuration**: Simple YAML-based configuration files
- **Environment Variables**: Override settings via environment variables
- **Runtime Validation**: Comprehensive configuration validation

## Project Structure

```
x-net/
├── lib/                      # Core network library
│   ├── acceptor.c/h          # TCP connection acceptor
│   ├── buffer.c/h            # Circular buffer implementation
│   ├── channel.c/h           # Event channel abstraction
│   ├── channel_map.c/h       # Channel mapping management
│   ├── config_loader.c/h     # Configuration file loader
│   ├── epoll_dispatcher.c    # epoll event dispatcher
│   ├── event_dispatcher.h    # Event dispatcher interface
│   ├── event_loop.c/h        # Main event loop implementation
│   ├── event_loop_thread.c/h # Event loop thread wrapper
│   ├── log.c/h               # Logging utilities
│   ├── tcp_connection.c/h    # TCP connection management
│   ├── tcp_server.c/h        # TCP server abstraction
│   ├── thread_pool.c/h       # Thread pool for multi-threading
│   └── utils.c/h             # Utility functions
│
├── http-server/              # HTTP server implementation
│   ├── http_request.c/h      # HTTP request parsing
│   ├── http_response.c/h     # HTTP response generation
│   ├── http_server.c/h       # HTTP server abstraction
│   ├── static_file.c/h       # Static file serving
│   └── main.c                # HTTP server example
│
├── tests/                    # Test suite
│   ├── test_buffer.c         # Buffer unit tests
│   ├── test_http.c           # HTTP unit tests
│   ├── test_config.c         # Configuration unit tests
│   ├── benchmark.c           # Performance benchmarks
│   └── CMakeLists.txt        # Test build configuration
│
├── src/                      # TCP server example
│   └── main.c                # Simple TCP echo server
│
├── config.yaml               # Default configuration file
├── README.md                 # This file
└── LICENSE                   # MIT License
```

## Building

### Prerequisites
- Linux operating system (epoll-based)
- GCC or Clang compiler
- CMake 3.16 or higher
- pthread library

### Build Instructions

```bash
# Build the HTTP server
cd http-server
mkdir build && cd build
cmake ..
make

# Build the TCP server example
cd ../src
mkdir build && cd build
cmake ..
make

# Build tests
cd ../../tests
mkdir build && cd build
cmake ..
make

# Run tests
ctest

# Run benchmarks
./benchmark
```

## Usage

### Basic HTTP Server

```c
#include "http_server.h"
#include "http_request.h"
#include "http_response.h"

int request_handler(struct http_request *req, struct http_response *resp) {
    // Get request method
    enum http_method method = http_request_get_method(req);

    // Get path
    if (http_request_path_matches(req, "/api/hello")) {
        if (method == HTTP_GET) {
            http_response_json(resp, "{\"message\":\"Hello, World!\"}");
        } else {
            http_response_set_status(resp, MethodNotAllowed, NULL);
        }
    }
    // Serve static files
    else {
        http_serve_static_file(req, resp, "./public");
    }

    return 0;
}

int main() {
    struct event_loop *ev_loop = event_loop_init();

    struct http_server *server = http_server_new(ev_loop, 3000, request_handler, 4);
    http_server_start(server);

    event_loop_run(ev_loop);

    return 0;
}
```

### Working with Configuration

```c
#include "config_loader.h"

int main() {
    // Create configuration with defaults
    struct server_config *config = config_new();

    // Load from file
    config_load_from_file(config, "config.yaml");

    // Override with environment variables
    config_load_from_env(config);

    // Validate configuration
    if (config_validate(config) != 0) {
        fprintf(stderr, "Invalid configuration\n");
        return 1;
    }

    // Use configuration
    printf("Server will listen on port %d\n", config->port);

    // Clean up
    config_free(config);

    return 0;
}
```

### HTTP Request Handling

```c
int handle_request(struct http_request *req, struct http_response *resp) {
    // Get query parameters
    char *user_id = http_request_get_param(req, "id");

    // Get cookies
    char *session = http_request_get_cookie(req, "session");

    // Get headers
    char *user_agent = http_request_get_header(req, "User-Agent");

    // Check request method
    switch (http_request_get_method(req)) {
        case HTTP_GET:
            // Handle GET
            break;
        case HTTP_POST:
            // Handle POST
            break;
        default:
            http_response_set_status(resp, MethodNotAllowed, NULL);
            return 0;
    }

    // Set response
    http_response_set_status(resp, OK, NULL);
    http_response_set_body(resp, strdup("Response body"));
    http_response_add_header(resp, strdup("Content-Type"), strdup("text/plain"));

    // Set cookie in response
    http_response_set_cookie(resp, "session", "abc123", 3600, "/", NULL, 0, 1);

    return 0;
}
```

### Static File Serving

```c
int handle_static_file(struct http_request *req, struct http_response *resp) {
    // Serve files from ./public directory
    return http_serve_static_file(req, resp, "./public");
}
```

## Configuration

The server can be configured through `config.yaml` or environment variables.

### Configuration File (config.yaml)

```yaml
# Server listening port (default: 3000)
port: 3000

# Server listening address (default: 0.0.0.0)
host: "0.0.0.0"

# Number of worker threads (default: 4)
thread_num: 4

# Connection timeout in seconds (default: 30)
timeout: 30

# Maximum number of concurrent connections (default: 1024)
max_connections: 1024

# Log file path (default: x-net.log)
log_file: "x-net.log"

# Log level: 0=ERROR, 1=WARN, 2=INFO, 3=DEBUG (default: 1)
log_level: 1

# Buffer size in bytes (default: 65535)
buffer_size: 65535

# Keep-alive timeout in seconds (default: 60)
keep_alive_timeout: 60

# Maximum request size in bytes (default: 10MB)
max_request_size: 10485760
```

### Environment Variables

- `X_NET_PORT`: Server port
- `X_NET_HOST`: Server host address
- `X_NET_THREAD_NUM`: Number of worker threads
- `X_NET_TIMEOUT`: Connection timeout
- `X_NET_MAX_CONNECTIONS`: Maximum connections
- `X_NET_LOG_FILE`: Log file path
- `X_NET_LOG_LEVEL`: Log level
- `X_NET_BUFFER_SIZE`: Buffer size

## Testing

### Running Unit Tests

```bash
cd tests/build
ctest

# Or run individual tests
./test_buffer
./test_http
./test_config
```

### Running Performance Benchmarks

```bash
cd tests/build
./benchmark
```

The benchmark suite tests:
- Buffer operations (append, read, char operations)
- HTTP operations (request/response creation, parsing)
- Header operations
- Mixed operations (complete request/response cycles)

## API Reference

### Buffer API

```c
// Create a new buffer
struct buffer *buffer_new();

// Free buffer
void buffer_free(struct buffer *buf);

// Append data to buffer
int buffer_append(struct buffer *buf, void *data, int size);

// Append string to buffer
int buffer_append_string(struct buffer *buf, char *s);

// Read data from buffer
int buffer_read(struct buffer *buf, void *data, int size);

// Get readable size
int buffer_readable_size(struct buffer *buf);

// Find CRLF in buffer
char *buffer_find_CRLF(struct buffer *buf);
```

### HTTP Request API

```c
// Create new request
struct http_request *http_request_new();

// Get HTTP method
enum http_method http_request_get_method(struct http_request *req);

// Get query parameter
char *http_request_get_param(struct http_request *req, const char *key);

// Get cookie
char *http_request_get_cookie(struct http_request *req, const char *name);

// Check path
int http_request_path_matches(struct http_request *req, const char *path);

// Get header
char *http_request_get_header(struct http_request *req, char *key);
```

### HTTP Response API

```c
// Create new response
struct http_response *http_response_new();

// Set status
void http_response_set_status(struct http_response *resp,
                             enum http_status_code code,
                             const char *message);

// Set body
void http_response_set_body(struct http_response *resp, char *body);

// Send JSON response
void http_response_json(struct http_response *resp, const char *json_data);

// Send HTML response
void http_response_html(struct http_response *resp, const char *html_data);

// Set cookie
void http_response_set_cookie(struct http_response *resp,
                             const char *name, const char *value,
                             int max_age, const char *path,
                             const char *domain, int secure, int httponly);

// Add header
void http_response_add_header(struct http_response *resp, char *key, char *value);
```

## Performance

X-Net is designed for high performance:

- **Event-Driven**: Non-blocking I/O with epoll
- **Thread Pool**: Efficient multi-threaded connection handling
- **Circular Buffer**: Zero-copy buffer operations
- **Memory Efficient**: Careful memory management and pooling

Typical performance on modern hardware:
- Handles thousands of concurrent connections
- Sub-millisecond request processing
- Efficient memory usage with automatic buffer management

## Security Considerations

- **Input Validation**: All inputs are validated before processing
- **Directory Traversal Protection**: Static file serving prevents directory traversal attacks
- **Connection Limits**: Configurable maximum connection limits
- **Timeout Handling**: Automatic connection timeout to prevent resource exhaustion

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by high-performance network frameworks like Netty, libevent, and muduo
- Built with epoll for efficient I/O multiplexing on Linux

## Contact

For questions, issues, or contributions, please visit the project repository.
