#include "buffer.h"
#include "common.h"
#include "utils.h"

const char *CRLF = "\r\n";

struct buffer *buffer_new() {
    struct buffer *buf = malloc(sizeof(struct buffer));
    if (!buf) {
        return NULL;
    }

    buf->data = malloc(INIT_BUFFER_SIZE);
    buf->read_index = 0;
    buf->write_index = 0;
    buf->total_size = INIT_BUFFER_SIZE;
    return buf;
}

void buffer_free(struct buffer *buf) {
    free(buf->data);
    free(buf);
}

int buffer_writeable_size(struct buffer *buf) {
    return buf->total_size - buf->write_index;
}

int buffer_readable_size(struct buffer *buf) {
    return buf->write_index - buf->read_index;
}

int buffer_front_spare_size(struct buffer *buf) {
    return buf->read_index;
}

void make_room(struct buffer *buf, int size) {
    if (buffer_writeable_size(buf) >= size) {
        return;
    }

    if (buffer_front_spare_size(buf) + buffer_writeable_size(buf) >= size) {
        int readable = buffer_readable_size(buf);
        int i;
        for (i = 0; i < readable; i++) {
            memcpy(buf->data + i, buf->data + buf->read_index + i, 1);
        }
        buf->read_index = 0;
        buf->write_index = readable;
    } else {
        void *tmp = realloc(buf->data, buf->total_size + size);
        if (tmp == NULL) {
            return;
        }
        buf->data = tmp;
        buf->total_size += size;
    }
}

// write data into buffer
int buffer_append(struct buffer *buf, void *data, int size) {
    if (data != NULL) {
        make_room(buf, size);
        memcpy(buf->data + buf->write_index, data, size);
        buf->write_index += size;
    }
}

// write a char into buffer
int buffer_append_char(struct buffer *buf, char c) {
    make_room(buf, 1);
    buf->data[buf->write_index++] = c;
}

// write a string into buffer
int buffer_append_string(struct buffer *buf, char *s) {
    if (s != NULL) {
        int size = strlen(s);
        buffer_append(buf, s, size);
    }
}

// write to buffer, the data is from socket
int buffer_socket_read(struct buffer *buf, int fd) {
    char additional_buffer[INIT_BUFFER_SIZE];
    struct iovec vec[2];
    int max_writable = buffer_writeable_size(buf);
    vec[0].iov_base = buf->data + buf->write_index;
    vec[0].iov_len = max_writable;
    vec[1].iov_base = additional_buffer;
    vec[1].iov_len = sizeof(additional_buffer);
    int result = readv(fd, vec, 2);
    if (result < 0) {
        return -1;
    } else if (result <= max_writable) {
        buf->write_index += result;
    } else {
        buf->write_index = buf->total_size;
        buffer_append(buf, additional_buffer, result - max_writable);
    }

    return result;
}

// read data from buffer
int buffer_read(struct buffer *buf, void *data, int size) {
    char c = buf->data[buf->read_index];
    buf->read_index++;
    return c;
}

// read a char from buffer
int buffer_read_char(struct buffer *buf) {
    char c = buf->data[buf->read_index];
    buf->read_index++;
    return c;
}

// find data from buffer
char *buffer_find_CRLF(struct buffer *buf) {
    int readable_size = buffer_readable_size(buf);
    if (readable_size < 0 || readable_size > buf->total_size) {
        perror("readable_size error");
        return NULL;
    }

    char *crlf = memmemx(buf->data + buf->read_index, readable_size, CRLF, 2);
    return crlf;
}