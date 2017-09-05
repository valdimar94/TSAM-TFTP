/* your code goes here. */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	int port_nr = argc;
	int socket_file_descriptor;
	struct sockaddr_in server, client;
	char message[512];

	socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

	if (socket_file_descriptor < 0)
		printf("An error has occured: failure to create an endpoint for communication.\n");

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	// the htons and htonl convert host byte order to network byte order
	// sin_port is stored as short, so htons is used
	// any port below 1024 requires superuser access so not sure what to do to test port 69?
	server.sin_port = htons(32000);
	// s_addr is stored as long so htonl is used
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind socket to a name with err check
	bind(socket_file_descriptor, (struct sockaddr *) &server, (socklen_t) sizeof(server));

	while(1){
		socklen_t len = (socklen_t) sizeof(client);
        ssize_t n = recvfrom(socket_file_descriptor, message, sizeof(message) - 1, 0, (struct sockaddr *) &client, &len);

        message[n] = '\0';
	printf("%d ", message[1]);
        // fprintf(stdout, "Received:\n%s\n", message[1]);
        fflush(stdout);

        sendto(socket_file_descriptor, message, (size_t) n, 0, (struct sockaddr *) &client, len);
	}

	return 0;
}
