// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>

// int main(int argc, char const *argv[])
// {
// 	// Create a new socket which will be used for listening to incoming connections.
// 	int socket_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
// 	if (socket_listen_fd < 0)
// 	{
// 		perror("An error occurred on listener socket creation");
// 		exit(EXIT_FAILURE);
// 	}

// 	// Bind the socket.
// 	struct sockaddr_in address;
// 	address.sin_family = AF_INET;
// 	address.sin_addr.s_addr = INADDR_ANY;
// 	address.sin_port = htons(8080);
// 	int address_len = sizeof(address);

// 	if (bind(socket_listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
// 	{
// 		perror("An error occurred on socket binding");
// 		exit(EXIT_FAILURE);
// 	}

// 	// Start listening for incoming requests.
// 	if (listen(socket_listen_fd, 10) < 0)
// 	{
// 		perror("An error occurred on socket listenning");
// 		exit(EXIT_FAILURE);
// 	}

// 	printf("Listening on port XXXX\r\n");

// 	// server_start(&socket_listen_fd, (struct sockaddr *)&address);//, address_len))

// 	while (1)
//     {
//         printf('While1\r\n');
//         // accept() blocks until a connection is received.
//         int socket_fd = accept(*socket, socket_address, (socklen_t *)&address_len);
//         if (socket_fd < 0)
//         {
//             perror("An error occurred accepting a connection");
//             continue;
//             // exit(EXIT_FAILURE);
//         }
//         printf('While2\r\n');

//         // Under load, we could accept a new connection before a newly created thread has had the chance to even access the socket.
//         // We will dynamically allocate a copy of the file descriptor, which the thread will have to free by itself.
//         int *socket_fd_copy = malloc(sizeof(int));
//         *socket_fd_copy = socket_fd;

//         pthread_create(&thread, NULL, handle_connection, socket_fd_copy);
//         pthread_detach(thread);
//     }


// 	exit(EXIT_SUCCESS);
// }

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>

#include "server.h"

int main(int argc, char *argv[]) {
	int portno = 5001;
	int queue = 10;

	// Create a new socket which will be used for listening to incoming connections.
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		perror("An error occurred creating a socket");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
    	perror("setsockopt(SO_REUSEADDR) failed");
	}

	// Initialize the socket structure.
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(portno);
	int address_len = sizeof(address);

	// Bind the socket
	if (bind(socket_fd, (struct sockaddr *) &address, address_len) < 0) {
		perror("An error occurred binding a socket");
		exit(EXIT_FAILURE);
	}

	// Start listening for incoming requests.
	if (listen(socket_fd, queue) < 0)
	{
		perror("An error occurred on socket listenning");
		exit(EXIT_FAILURE);
	}

	server_start(&socket_fd);

	close(socket_fd);
}