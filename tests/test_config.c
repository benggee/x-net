#include "../lib/config_loader.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_config_new() {
    struct server_config *config = config_new();
    assert(config != NULL);
    assert(config->port == 3000);
    assert(config->thread_num == 4);
    assert(config->timeout == 30);
    assert(config->max_connections == 1024);
    assert(strcmp(config->host, "0.0.0.0") == 0);
    assert(strcmp(config->log_file, "x-net.log") == 0);

    config_free(config);
    printf("test_config_new: PASSED\n");
}

void test_config_validate() {
    struct server_config *config = config_new();

    // Valid configuration
    assert(config_validate(config) == 0);

    // Invalid port
    config->port = -1;
    assert(config_validate(config) == -1);
    config->port = 70000;
    assert(config_validate(config) == -1);
    config->port = 8080; // Valid

    // Invalid thread number
    config->thread_num = -1;
    assert(config_validate(config) == -1);
    config->thread_num = 300;
    assert(config_validate(config) == -1);
    config->thread_num = 4; // Valid

    // Invalid timeout
    config->timeout = -1;
    assert(config_validate(config) == -1);
    config->timeout = 30; // Valid

    // Invalid max connections
    config->max_connections = 0;
    assert(config_validate(config) == -1);
    config->max_connections = 1024; // Valid

    // Invalid buffer size
    config->buffer_size = 100;
    assert(config_validate(config) == -1);
    config->buffer_size = 65535; // Valid

    config_free(config);
    printf("test_config_validate: PASSED\n");
}

void test_config_load_from_file() {
    struct server_config *config = config_new();

    // Create a test config file
    FILE *f = fopen("/tmp/test_x_net_config.yaml", "w");
    assert(f != NULL);
    fprintf(f, "# Test configuration\n");
    fprintf(f, "port: 8080\n");
    fprintf(f, "host: \"127.0.0.1\"\n");
    fprintf(f, "thread_num: 8\n");
    fprintf(f, "timeout: 60\n");
    fprintf(f, "max_connections: 2048\n");
    fprintf(f, "log_file: \"/tmp/test.log\"\n");
    fprintf(f, "log_level: 2\n");
    fprintf(f, "buffer_size: 131072\n");
    fclose(f);

    // Load configuration
    int result = config_load_from_file(config, "/tmp/test_x_net_config.yaml");
    assert(result == 0);
    assert(config->port == 8080);
    assert(strcmp(config->host, "127.0.0.1") == 0);
    assert(config->thread_num == 8);
    assert(config->timeout == 60);
    assert(config->max_connections == 2048);
    assert(strcmp(config->log_file, "/tmp/test.log") == 0);
    assert(config->log_level == 2);
    assert(config->buffer_size == 131072);

    // Clean up
    remove("/tmp/test_x_net_config.yaml");
    config_free(config);
    printf("test_config_load_from_file: PASSED\n");
}

void test_config_load_from_env() {
    struct server_config *config = config_new();

    // Set environment variables
    setenv("X_NET_PORT", "9000", 1);
    setenv("X_NET_THREAD_NUM", "16", 1);
    setenv("X_NET_HOST", "localhost", 1);
    setenv("X_NET_LOG_FILE", "/var/log/x-net.log", 1);

    // Load from environment
    int result = config_load_from_env(config);
    assert(result == 0);
    assert(config->port == 9000);
    assert(config->thread_num == 16);
    assert(strcmp(config->host, "localhost") == 0);
    assert(strcmp(config->log_file, "/var/log/x-net.log") == 0);

    // Clean up
    unsetenv("X_NET_PORT");
    unsetenv("X_NET_THREAD_NUM");
    unsetenv("X_NET_HOST");
    unsetenv("X_NET_LOG_FILE");
    config_free(config);
    printf("test_config_load_from_env: PASSED\n");
}

void test_config_to_string() {
    struct server_config *config = config_new();
    config->port = 8080;

    char *str = config_to_string(config);
    assert(str != NULL);
    assert(strstr(str, "8080") != NULL);
    assert(strstr(str, "Server Configuration") != NULL);

    free(str);
    config_free(config);
    printf("test_config_to_string: PASSED\n");
}

void test_config_null_checks() {
    // Test NULL handling
    assert(config_validate(NULL) == -1);
    assert(config_load_from_file(NULL, "test.yaml") == -1);
    assert(config_load_from_env(NULL) == -1);
    assert(config_to_string(NULL) == NULL);

    printf("test_config_null_checks: PASSED\n");
}

int main() {
    printf("=== Config Unit Tests ===\n");

    test_config_new();
    test_config_validate();
    test_config_load_from_file();
    test_config_load_from_env();
    test_config_to_string();
    test_config_null_checks();

    printf("\n=== All Config Tests Passed ===\n");
    return 0;
}
