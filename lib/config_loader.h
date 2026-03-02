#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <stddef.h>

// Server configuration structure
struct server_config {
    int port;
    int thread_num;
    int timeout;
    int max_connections;
    char *host;
    char *log_file;
    int log_level;
    int buffer_size;
    int keep_alive_timeout;
    int max_request_size;
};

// Initialize configuration with default values
struct server_config *config_new(void);

// Load configuration from YAML file
int config_load_from_file(struct server_config *config, const char *filename);

// Load configuration from environment variables (overrides file config)
int config_load_from_env(struct server_config *config);

// Free configuration resources
void config_free(struct server_config *config);

// Validate configuration
int config_validate(struct server_config *config);

// Get configuration value as string
char *config_to_string(struct server_config *config);

#endif // CONFIG_LOADER_H
