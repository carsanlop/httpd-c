#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>

#include "thpool.h"
// #include "file.h"
#include "server.h"

void init_signals();
void server_start(int*);
void server_stop(int);
void server_handle_connection(void *params);
void server_handle_request(int socket_fd, char *request);
char* get_status_line();
char* get_content_length(char *content);
void send_ok(int socket_fd, char *content);
void send_not_found(int socket_fd, char *content);
int is_valid_method(char *method);

static volatile sig_atomic_t server_loop = 1;

void init_signals() {
	struct sigaction signal_action;
	signal_action.sa_flags = 0; // Don't restart the blocking call after it failed with EINTR
	signal_action.sa_handler = server_stop;
	sigemptyset(&signal_action.sa_mask);
	sigfillset(&signal_action.sa_mask); // Block every signal during the handler is executing
	if (sigaction(SIGINT, &signal_action, NULL) < 0) {
		perror("error handling SIGINT");
	}
	if (sigaction(SIGTERM, &signal_action, NULL) < 0) {
		perror("error handling SIGTERM");
	}
	if (sigaction(SIGQUIT, &signal_action, NULL) < 0) {
		perror("error handling SIGQUIT");
	}
}

/**
 * @brief  Initialize server
 *
 * Initializes a server. This function will not return until a signal has been received.
 *
 * @example
 *
 *    ..
 *    threadpool thpool;                     //First we declare a threadpool
 *    thpool = thpool_init(4);               //then we initialize it to 4 threads
 *    ..
 *
 * @param  listen_fd		pointer to the listen socket
 */
void server_start(int *listen_fd)
{
	init_signals();

	struct sockaddr_in connection_addr;
	socklen_t connection_addr_len = sizeof(connection_addr);

	threadpool thpool = thpool_init(4);

	while (server_loop)
	{
		int connection_fd = accept(*listen_fd, (struct sockaddr *) &connection_addr, &connection_addr_len);
		if (connection_fd < 0)
		{
			// Interrupted system call
			if (errno == EINTR) {
				continue;
			}

			perror("An error occurred accepting a connection");
			exit(EXIT_FAILURE);
		}

		// Create a copy of the fd.
		// Otherwise, under heavy load, a new connection could be incoming before the thread pool has time to execute.
		// That would cause the fd to already be overriden.
		int *socket_fd_copy = malloc(sizeof(int));
		*socket_fd_copy = connection_fd;
		thpool_add_work(thpool, server_handle_connection, socket_fd_copy);
	}

	thpool_wait(thpool);
	thpool_destroy(thpool);
}

void server_stop (int signum)
{
    server_loop = 0;
}

/**
 * @brief Handles an incoming connection.
 * 
 * @param params A pointer to the socket fd.
 */
void server_handle_connection(void *params)
{
	// Retrieve the original fd and free it (caller will not do it).
	int socket_fd = *((int*) params);
	free(params);

	// TODO Get remote address

	// Read the request.
	char buffer[1024] = {0};
	ssize_t bytes_read = read(socket_fd, buffer, sizeof(buffer));
	// printf("%s\n", buffer);
	server_handle_request(socket_fd, buffer);

	close(socket_fd);
}

/**
 * @brief Handles an incoming request.
 *
 * @param   socket_fd  Descriptor where the response will be written to.
 * @param   content Content of the request.
 */
void server_handle_request(int socket_fd, char *request)
{
	#define DELIMITER " "

    // The very first line will contain the basic information: method, resource, and protocol.
    // For example: GET /hello.html HTTP/1.1
    char *line = strtok(request, "\n");
    char *tokens;

    tokens = strtok(line, DELIMITER);
    char *method = tokens;
	if (is_valid_method(method) < 1) {
		// TODO Send 415 or a similar error instead of simply returning.
		return;
	}

    tokens = strtok(NULL, DELIMITER);
    char *resource = tokens;

    tokens = strtok(NULL, DELIMITER);
    char *protocol = tokens;

	send_ok(socket_fd, "Saludos terrícola");
	return;

    // After the request has been read, we know which resource we require, so it can be loaded.
    // It could be a non-existent resource, in which case, content will be NULL.
    char *content = NULL;
//     read_resource(resource, &content);

    if (content == NULL)
    {
        // printf("404 Not Found           %s\n", resource);
        send_not_found(socket_fd, NULL);
    }
    else
    {
        // printf("200 OK                  %s\n", resource);
        send_ok(socket_fd, content);
    }

    free(content);
}

char* get_status_line() {
	size_t buffer_size = sizeof(char) * 128;
	char *buffer = malloc(buffer_size);

	char *protocol = "HTTP/1.1";
	char *status_code = "200";
	char *status_text = "OK";

	// for (int i = 0; i < sizeof(status_line); i++) {
	// 	status_line[i] = 'A';
    // }
	// printf("%ld\n", sizeof(status_line));
	// printf("%s\n", status_line);

	// strcpy(status_line, protocol);
	// strcpy(status_line, status_code);
	// strcpy(status_line, status_text);
	// strncat(status_line, protocol, strlen(protocol));
	// strncat(status_line, status_code, strlen(status_code));
	// strncat(status_line, status_text, strlen(status_text));
	// printf("%s\n", status_line);
	

	snprintf(buffer, buffer_size, "%s %s %s\r\n", protocol, status_code, status_text);

	return buffer;
}

char* get_content_length(char *content) {
	size_t buffer_size = sizeof(char) * 64;
	char *buffer = malloc(buffer_size);

	snprintf(buffer, buffer_size, "Content-Length: %ld\r\n\r\n", strlen(content));

	return buffer;
}

void send_ok(int socket_fd, char *content)
{
	size_t buffer_size = sizeof(char) * 1024;
	char *buffer = malloc(buffer_size);

	char *status_line = get_status_line();
	char *headers = "Content-Type: text/html\r\n";
	char *connection = "Connection: close\r\n";
	char *content_length = get_content_length(content);

	snprintf(buffer, buffer_size, "%s%s%s%s%s", status_line, headers, connection, content_length, content);

	write(socket_fd, buffer, strlen(buffer));
	
	free(status_line);
	free(content_length);
	free(buffer);
}

void send_not_found(int socket_fd, char *content)
{
    const char *response_body = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nConnection: close\r\nContent-Length: 0";
    write(socket_fd, response_body, strlen(response_body));
}

int is_valid_method(char *method) {
	return 1;
}