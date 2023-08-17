#ifndef BUFFER_H
#define BUFFER_H

#define INIT_BUFFER_SIZE 65535

struct buffer {
    char *data;
    int read_index;
    int write_index;
    int total_size;
};

struct buffer *buffer_new();

void buffer_free(struct buffer *buf);

int buffer_writeable_size(struct buffer *buf);

int buffer_readable_size(struct buffer *buf);

int buffer_front_spare_size(struct buffer *buf);

// write data into buffer
int buffer_append(struct buffer *buf, void *data, int size);

// write a char into buffer
int buffer_append_char(struct buffer *buf, char c);

// write a string into buffer
int buffer_append_string(struct buffer *buf, char *s);

// write to buffer, the data is from socket
int buffer_socket_read(struct buffer *buf, int fd);

// read data from buffer
int buffer_read(struct buffer *buf, void *data, int size);

// read a char from buffer
int buffer_read_char(struct buffer *buf);

// find data from buffer
char *buffer_find_CRLF(struct buffer *buf);

#endif