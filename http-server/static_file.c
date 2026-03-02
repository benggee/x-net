#include "static_file.h"
#include "common.h"
#include "log.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Common MIME types
static const struct mime_type mime_types[] = {
    {".html", "text/html"},
    {".htm", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif", "image/gif"},
    {".svg", "image/svg+xml"},
    {".ico", "image/x-icon"},
    {".txt", "text/plain"},
    {".pdf", "application/pdf"},
    {".xml", "application/xml"},
    {".zip", "application/zip"},
    {".woff", "font/woff"},
    {".woff2", "font/woff2"},
    {".ttf", "font/ttf"},
    {".eot", "application/vnd.ms-fontobject"},
    {NULL, "application/octet-stream"}
};

const char *get_mime_type(const char *path) {
    if (!path) {
        return "application/octet-stream";
    }

    const char *ext = strrchr(path, '.');
    if (!ext) {
        return "text/plain";
    }

    for (int i = 0; mime_types[i].extension != NULL; i++) {
        if (strcasecmp(ext, mime_types[i].extension) == 0) {
            return mime_types[i].mime_type;
        }
    }

    return "application/octet-stream";
}

// URL decode function
void url_decode(char *dst, const char *src, size_t len) {
    char a, b;
    while (*src && len > 0) {
        if (*src == '%' && (a = src[1]) && (b = src[2]) && isxdigit(a) && isxdigit(b)) {
            if (a >= 'a') {
                a -= 'a' - 'A';
            }
            if (a >= 'A') {
                a -= ('A' - 10);
            } else {
                a -= '0';
            }
            if (b >= 'a') {
                b -= 'a' - 'A';
            }
            if (b >= 'A') {
                b -= ('A' - 10);
            } else {
                b -= '0';
            }
            *dst++ = 16 * a + b;
            src += 3;
            len -= 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
            len--;
        } else {
            *dst++ = *src++;
            len--;
        }
    }
    *dst = '\0';
}

// Check for directory traversal attacks
int is_safe_path(const char *path) {
    if (!path) {
        return 0;
    }

    // Check for ".." in path
    if (strstr(path, "..") != NULL) {
        return 0;
    }

    // Check for absolute path
    if (path[0] == '/') {
        return 0;
    }

    return 1;
}

int http_serve_static_file(struct http_request *req, struct http_response *resp, const char *root_dir) {
    if (!req || !resp || !root_dir) {
        return -1;
    }

    if (!req->path) {
        resp->status_code = NotFound;
        resp->status_message = "Not Found";
        resp->body = strdup("<html><body>404 Not Found</body></html>");
        return 0;
    }

    // Check if path is safe
    if (!is_safe_path(req->path)) {
        resp->status_code = BadRequest;
        resp->status_message = "Bad Request";
        resp->body = strdup("<html><body>400 Bad Request</body></html>");
        return 0;
    }

    // URL decode the path
    char decoded_path[MAX_PATH_LENGTH];
    url_decode(decoded_path, req->path, sizeof(decoded_path) - 1);

    // Build full file path
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s%s", root_dir, decoded_path);

    // Try to open file
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        // If it's a directory, try to serve index.html
        struct stat st;
        if (stat(file_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            snprintf(file_path, sizeof(file_path), "%s%s/index.html", root_dir, decoded_path);
            fd = open(file_path, O_RDONLY);
        }

        if (fd < 0) {
            resp->status_code = NotFound;
            resp->status_message = "Not Found";
            resp->body = strdup("<html><body><h1>404 Not Found</h1></body></html>");
            return 0;
        }
    }

    // Get file size
    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        resp->status_code = NotFound;
        resp->status_message = "Not Found";
        resp->body = strdup("<html><body>404 Not Found</body></html>");
        return 0;
    }

    off_t file_size = st.st_size;

    // Allocate buffer for file content
    char *file_content = malloc(file_size + 1);
    if (!file_content) {
        close(fd);
        resp->status_code = NotFound;
        resp->status_message = "Internal Server Error";
        resp->body = strdup("<html><body>500 Internal Server Error</body></html>");
        return 0;
    }

    // Read file content
    ssize_t bytes_read = read(fd, file_content, file_size);
    close(fd);

    if (bytes_read != file_size) {
        free(file_content);
        resp->status_code = NotFound;
        resp->status_message = "Internal Server Error";
        resp->body = strdup("<html><body>500 Internal Server Error</body></html>");
        return 0;
    }

    file_content[file_size] = '\0';

    // Set response
    resp->status_code = OK;
    resp->status_message = "OK";
    resp->body = file_content;

    // Set content type
    char content_type[128];
    snprintf(content_type, sizeof(content_type), "Content-Type: %s", get_mime_type(file_path));
    http_response_add_header(resp, strdup("Content-Type"), strdup(get_mime_type(file_path)));

    x_msgx("Served static file: %s (size: %ld)", file_path, file_size);
    return 0;
}

// Helper function to add headers to response
void http_response_add_header(struct http_response *resp, char *key, char *value) {
    if (!resp || !key || !value) {
        return;
    }

    // Check if we have space for more headers
    if (resp->response_headers_number >= 128) {
        return;
    }

    resp->response_headers[resp->response_headers_number].key = key;
    resp->response_headers[resp->response_headers_number].value = value;
    resp->response_headers_number++;
}
