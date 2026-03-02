#ifndef STATIC_FILE_H
#define STATIC_FILE_H

#include "http_request.h"
#include "http_response.h"

// MIME type structure
struct mime_type {
    const char *extension;
    const char *mime_type;
};

// Serve a static file
// Returns 0 on success, -1 on error
int http_serve_static_file(struct http_request *req, struct http_response *resp, const char *root_dir);

// Get MIME type for a file extension
const char *get_mime_type(const char *path);

// URL decode a string
void url_decode(char *dst, const char *src, size_t len);

// Check if a file path is safe (no directory traversal)
int is_safe_path(const char *path);

#endif // STATIC_FILE_H
