#include "http_request.h"
#include "http_server.h"
#include "http_response.h"
#include "utils.h"

int on_request(struct http_request *http_req, struct http_response *http_reply) {
    char *url = http_req->url;
    char *question = memmemx(url, strlen(url), "?", 1);
    char *path = NULL;
    if (question != NULL) {
        path = malloc(question - url);
        strncpy(path, url, question - url);
    } else {
        path = malloc(strlen(url));
        strncpy(path, url, strlen(url));
    }

    if (strcmp(path, "/") == 0) {
        http_reply->status_code = OK;
        http_reply->status_message = "OK";
        http_reply->content_type = "text/html";
        http_reply->body = "<html><head><title>x-net</title></head><body><h1>Hello, x-net</h1></body></html>";
    } else if (strcmp(path, "/network") == 0) {
        http_reply->status_code = OK;
        http_reply->status_message = "OK";
        http_reply->content_type = "text/plain";
        http_reply->body = "hello, x-net";
    } else {
        http_reply->status_code = NotFound;
        http_reply->status_message = "Not Found";
        http_reply->keep_connected = 1;
    }

    return 0;
}

int main() {
    struct event_loop *ev_loop = event_loop_init();

    struct http_server *http_serv = http_server_new(ev_loop, SERV_PORT, on_request, 2);
    http_server_start(http_serv);

    event_loop_run(ev_loop);
}