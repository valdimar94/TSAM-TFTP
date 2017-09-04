/* your code goes here. */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	int socket_file_descriptor;
	struct sockaddr_in server, client;

	socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

	if (socket_file_descriptor < 0)
		printf("An error has occured, failure to create an endpoint for communication.");

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	// the htons and htonl convert host byte order to network byte order
	// sin_port is stored as short, so htons is used
	server.sin_port = htons(69);
	// s_addr is stored as long so htonl is used
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	printf("Server has stopped!\n");
	return 0;
}
