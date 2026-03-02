#include "config_loader.h"
#include "common.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Default configuration values
#define DEFAULT_PORT 3000
#define DEFAULT_THREAD_NUM 4
#define DEFAULT_TIMEOUT 30
#define DEFAULT_MAX_CONNECTIONS 1024
#define DEFAULT_HOST "0.0.0.0"
#define DEFAULT_LOG_FILE "x-net.log"
#define DEFAULT_LOG_LEVEL 1
#define DEFAULT_BUFFER_SIZE 65535
#define DEFAULT_KEEP_ALIVE_TIMEOUT 60
#define DEFAULT_MAX_REQUEST_SIZE 10485760

struct server_config *config_new(void) {
    struct server_config *config = malloc(sizeof(struct server_config));
    if (!config) {
        return NULL;
    }

    // Set default values
    config->port = DEFAULT_PORT;
    config->thread_num = DEFAULT_THREAD_NUM;
    config->timeout = DEFAULT_TIMEOUT;
    config->max_connections = DEFAULT_MAX_CONNECTIONS;
    config->host = strdup(DEFAULT_HOST);
    config->log_file = strdup(DEFAULT_LOG_FILE);
    config->log_level = DEFAULT_LOG_LEVEL;
    config->buffer_size = DEFAULT_BUFFER_SIZE;
    config->keep_alive_timeout = DEFAULT_KEEP_ALIVE_TIMEOUT;
    config->max_request_size = DEFAULT_MAX_REQUEST_SIZE;

    if (!config->host || !config->log_file) {
        config_free(config);
        return NULL;
    }

    return config;
}

// Simple YAML-like key-value parser
static char *trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }

    if (*str == 0) {
        return str;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }

    end[1] = '\0';
    return str;
}

// Remove quotes from string
static char *remove_quotes(char *str) {
    if (!str) {
        return NULL;
    }

    size_t len = strlen(str);
    if (len >= 2 && ((str[0] == '"' && str[len-1] == '"') ||
                      (str[0] == '\'' && str[len-1] == '\''))) {
        str[len-1] = '\0';
        return str + 1;
    }
    return str;
}

// Parse a single line of config
static void parse_config_line(struct server_config *config, char *line) {
    if (!line || !config || line[0] == '#' || line[0] == '\0') {
        return;
    }

    // Skip lines without colon
    char *colon = strchr(line, ':');
    if (!colon) {
        return;
    }

    // Split key and value
    *colon = '\0';
    char *key = trim_whitespace(line);
    char *value = trim_whitespace(colon + 1);

    // Remove quotes from value
    value = remove_quotes(value);

    // Parse key-value pairs
    if (strcmp(key, "port") == 0) {
        config->port = atoi(value);
    } else if (strcmp(key, "thread_num") == 0) {
        config->thread_num = atoi(value);
    } else if (strcmp(key, "timeout") == 0) {
        config->timeout = atoi(value);
    } else if (strcmp(key, "max_connections") == 0) {
        config->max_connections = atoi(value);
    } else if (strcmp(key, "host") == 0) {
        free(config->host);
        config->host = strdup(value);
    } else if (strcmp(key, "log_file") == 0) {
        free(config->log_file);
        config->log_file = strdup(value);
    } else if (strcmp(key, "log_level") == 0) {
        config->log_level = atoi(value);
    } else if (strcmp(key, "buffer_size") == 0) {
        config->buffer_size = atoi(value);
    } else if (strcmp(key, "keep_alive_timeout") == 0) {
        config->keep_alive_timeout = atoi(value);
    } else if (strcmp(key, "max_request_size") == 0) {
        config->max_request_size = atoi(value);
    }
}

int config_load_from_file(struct server_config *config, const char *filename) {
    if (!config || !filename) {
        return -1;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        x_errx("Failed to open config file: %s", filename);
        return -1;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char *trimmed = trim_whitespace(line);
        parse_config_line(config, trimmed);
    }

    fclose(file);
    x_msgx("Loaded configuration from: %s", filename);
    return 0;
}

int config_load_from_env(struct server_config *config) {
    if (!config) {
        return -1;
    }

    const char *env_port = getenv("X_NET_PORT");
    if (env_port) {
        config->port = atoi(env_port);
    }

    const char *env_thread_num = getenv("X_NET_THREAD_NUM");
    if (env_thread_num) {
        config->thread_num = atoi(env_thread_num);
    }

    const char *env_timeout = getenv("X_NET_TIMEOUT");
    if (env_timeout) {
        config->timeout = atoi(env_timeout);
    }

    const char *env_max_conn = getenv("X_NET_MAX_CONNECTIONS");
    if (env_max_conn) {
        config->max_connections = atoi(env_max_conn);
    }

    const char *env_host = getenv("X_NET_HOST");
    if (env_host) {
        free(config->host);
        config->host = strdup(env_host);
    }

    const char *env_log_file = getenv("X_NET_LOG_FILE");
    if (env_log_file) {
        free(config->log_file);
        config->log_file = strdup(env_log_file);
    }

    const char *env_log_level = getenv("X_NET_LOG_LEVEL");
    if (env_log_level) {
        config->log_level = atoi(env_log_level);
    }

    const char *env_buffer_size = getenv("X_NET_BUFFER_SIZE");
    if (env_buffer_size) {
        config->buffer_size = atoi(env_buffer_size);
    }

    x_msgx("Loaded configuration from environment variables");
    return 0;
}

void config_free(struct server_config *config) {
    if (!config) {
        return;
    }

    if (config->host) {
        free(config->host);
    }
    if (config->log_file) {
        free(config->log_file);
    }
    free(config);
}

int config_validate(struct server_config *config) {
    if (!config) {
        return -1;
    }

    // Validate port range
    if (config->port < 1 || config->port > 65535) {
        x_errx("Invalid port number: %d", config->port);
        return -1;
    }

    // Validate thread number
    if (config->thread_num < 0 || config->thread_num > 256) {
        x_errx("Invalid thread number: %d", config->thread_num);
        return -1;
    }

    // Validate timeout
    if (config->timeout < 0) {
        x_errx("Invalid timeout: %d", config->timeout);
        return -1;
    }

    // Validate max connections
    if (config->max_connections < 1) {
        x_errx("Invalid max connections: %d", config->max_connections);
        return -1;
    }

    // Validate host
    if (!config->host || strlen(config->host) == 0) {
        x_errx("Invalid host address");
        return -1;
    }

    // Validate buffer size
    if (config->buffer_size < 1024) {
        x_errx("Invalid buffer size: %d", config->buffer_size);
        return -1;
    }

    x_msgx("Configuration validated successfully");
    return 0;
}

char *config_to_string(struct server_config *config) {
    if (!config) {
        return NULL;
    }

    char *str = malloc(1024);
    if (!str) {
        return NULL;
    }

    snprintf(str, 1024,
             "Server Configuration:\n"
             "  Port: %d\n"
             "  Host: %s\n"
             "  Thread Num: %d\n"
             "  Timeout: %d\n"
             "  Max Connections: %d\n"
             "  Log File: %s\n"
             "  Log Level: %d\n"
             "  Buffer Size: %d\n"
             "  Keep Alive Timeout: %d\n"
             "  Max Request Size: %d\n",
             config->port,
             config->host,
             config->thread_num,
             config->timeout,
             config->max_connections,
             config->log_file,
             config->log_level,
             config->buffer_size,
             config->keep_alive_timeout,
             config->max_request_size);

    return str;
}
