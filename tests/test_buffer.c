#include "../lib/buffer.h"
#include "../lib/buffer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_buffer_new() {
    struct buffer *buf = buffer_new();
    assert(buf != NULL);
    assert(buf->data != NULL);
    assert(buf->total_size == INIT_BUFFER_SIZE);
    assert(buf->read_index == 0);
    assert(buf->write_index == 0);

    buffer_free(buf);
    printf("test_buffer_new: PASSED\n");
}

void test_buffer_append() {
    struct buffer *buf = buffer_new();
    const char *test_data = "Hello, World!";
    int size = strlen(test_data);

    int result = buffer_append(buf, (void *)test_data, size);
    assert(result == size);
    assert(buf->write_index == size);
    assert(memcmp(buf->data, test_data, size) == 0);

    buffer_free(buf);
    printf("test_buffer_append: PASSED\n");
}

void test_buffer_append_string() {
    struct buffer *buf = buffer_new();
    const char *test_str = "Test String";

    int result = buffer_append_string(buf, (char *)test_str);
    assert(result == strlen(test_str));
    assert(buf->write_index == strlen(test_str));

    buffer_free(buf);
    printf("test_buffer_append_string: PASSED\n");
}

void test_buffer_append_char() {
    struct buffer *buf = buffer_new();

    int result = buffer_append_char(buf, 'A');
    assert(result == 1);
    assert(buf->write_index == 1);
    assert(buf->data[0] == 'A');

    buffer_free(buf);
    printf("test_buffer_append_char: PASSED\n");
}

void test_buffer_readable_size() {
    struct buffer *buf = buffer_new();
    buffer_append_string(buf, "Test");

    int readable = buffer_readable_size(buf);
    assert(readable == 4);

    buffer_free(buf);
    printf("test_buffer_readable_size: PASSED\n");
}

void test_buffer_writeable_size() {
    struct buffer *buf = buffer_new();

    int writable = buffer_writeable_size(buf);
    assert(writable == INIT_BUFFER_SIZE);

    buffer_append_string(buf, "Test");
    writable = buffer_writeable_size(buf);
    assert(writable == INIT_BUFFER_SIZE - 4);

    buffer_free(buf);
    printf("test_buffer_writeable_size: PASSED\n");
}

void test_buffer_read() {
    struct buffer *buf = buffer_new();
    const char *test_data = "Test Data";
    buffer_append(buf, (void *)test_data, strlen(test_data));

    char read_data[20];
    int bytes_read = buffer_read(buf, read_data, 4);
    assert(bytes_read == 4);
    assert(memcmp(read_data, "Test", 4) == 0);
    assert(buf->read_index == 4);

    buffer_free(buf);
    printf("test_buffer_read: PASSED\n");
}

void test_buffer_read_char() {
    struct buffer *buf = buffer_new();
    buffer_append_char(buf, 'X');

    int c = buffer_read_char(buf);
    assert(c == 'X');
    assert(buf->read_index == 1);

    buffer_free(buf);
    printf("test_buffer_read_char: PASSED\n");
}

void test_buffer_find_CRLF() {
    struct buffer *buf = buffer_new();
    buffer_append_string(buf, "Hello\r\nWorld");

    char *crlf = buffer_find_CRLF(buf);
    assert(crlf != NULL);
    assert(crlf == buf->data + 5);

    buffer_free(buf);
    printf("test_buffer_find_CRLF: PASSED\n");
}

void test_buffer_make_room() {
    struct buffer *buf = buffer_new();
    buf->write_index = buf->total_size - 10;

    int old_size = buf->total_size;
    buffer_append_char(buf, 'A');
    assert(buf->total_size > old_size);

    buffer_free(buf);
    printf("test_buffer_make_room: PASSED\n");
}

void test_buffer_edge_cases() {
    struct buffer *buf = buffer_new();

    // Test NULL checks
    assert(buffer_append(NULL, "test", 4) == -1);
    assert(buffer_append(buf, NULL, 4) == -1);
    assert(buffer_append_string(NULL, "test") == -1);
    assert(buffer_append_string(buf, NULL) == -1);

    // Test empty reads
    char data[10];
    assert(buffer_read(buf, data, 10) == 0);
    assert(buffer_read_char(buf) == -1);

    buffer_free(buf);
    printf("test_buffer_edge_cases: PASSED\n");
}

int main() {
    printf("=== Buffer Unit Tests ===\n");

    test_buffer_new();
    test_buffer_append();
    test_buffer_append_string();
    test_buffer_append_char();
    test_buffer_readable_size();
    test_buffer_writeable_size();
    test_buffer_read();
    test_buffer_read_char();
    test_buffer_find_CRLF();
    test_buffer_make_room();
    test_buffer_edge_cases();

    printf("\n=== All Buffer Tests Passed ===\n");
    return 0;
}
