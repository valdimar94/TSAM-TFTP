/* your code goes here. */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define RRQ 1


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
	server.sin_port = htons(32001);
	// s_addr is stored as long so htonl is used
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind socket to a name with err check
	int x = bind(socket_file_descriptor, (struct sockaddr *) &server, (socklen_t) sizeof(server));
	// IF PORT IS IN USE
	if (x < 0) {
		printf("Could not bind man.... soorry man");
		return 0;
	}
	
	while(1){
		socklen_t len = (socklen_t) sizeof(client);
        ssize_t n = recvfrom(socket_file_descriptor, message, sizeof(message) - 1, 0, (struct sockaddr *) &client, &len);
	
        //message[n] = '\0';
	//printf("%d ", message[1]);
        
	// the second byte in the array contains the Opcode
	if (message[1] == RRQ)
	{	
		// We jump to over the first two bytes to get the filename
		char* file_name = message + 2;
		// Length of given file name
		size_t f_n_length = strlen(file_name);
		printf("%s\n", file_name);
		
		// Jump over opcode, filename and null terminator to get the mode of transfer.
		char *mode = message + f_n_length + 3;
		printf("%s\n", mode);
		
		// TEST:::
		size_t argv_length = strlen(argv[2]);
		
		// Initialize array for data path
		char full_path[argv_length + f_n_length + 2];
		
		strcpy(full_path, "lalala");
		//strcpy(full_path, file_name);

		printf("%d\n", argv_length);

		fprintf(stdout, "Path: %s\n", full_path); fflush(stdout);
		
				
	}
	else{
		printf("Ohhhhh, now you fucked uup!");
	}
        fflush(stdout);

        sendto(socket_file_descriptor, message, (size_t) n, 0, (struct sockaddr *) &client, len);
	}

	return 0;
}
