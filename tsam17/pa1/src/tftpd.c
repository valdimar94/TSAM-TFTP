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
	if (bind(socket_file_descriptor, (struct sockaddr *)&server, (socklen_t)sizeof(server)) < 0)
	{
		// IF PORT IS IN USE
		printf("Could not bind to port");
		return 0;
	}

	while (1)
	{
		socklen_t len = (socklen_t)sizeof(client);
		ssize_t n = recvfrom(socket_file_descriptor, message, sizeof(message) - 1, 0, (struct sockaddr *)&client, &len);
		if (message[1] == 2)
		{
			printf("Opcode 2");
		}
		if (message[1] == 3)
		{
			printf("Opcode 3");
		}
		if (message[1] == 4)
		{
			printf("Opcode 4");
		}
		if (message[1] == 5)
		{
			printf("Opcode 5");
		}
		// the second byte in the array contains the Opcode
		if (message[1] == RRQ)
		{
			// We jump to over the first two bytes to get the filename
			char *file_name = message + 2;
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

			strcpy(full_path, argv[2]);
			strcpy(full_path + argv_length, "/");
			strcpy(full_path + argv_length + 1, file_name);

			fprintf(stdout, "Path: %s\n", full_path);
			fflush(stdout);

			FILE *file;
			file = fopen(full_path, "r");

			if (file == NULL)
			{
				printf("File does not exist, need to send msg to client!");
			}
			// Keep track of length of file
			fseek(file, 0, SEEK_END);
			size_t file_size = ftell(file);
			// Go back to the beginning of the file
			fseek(file, 0, SEEK_SET);

			int curr_file_size;
			char buffer[512];
			char full_message[516];
			memset(buffer, 0, sizeof buffer);
			int break_next = 0;
			unsigned short package_nr = 1;
			while (1)
			{
				int retry_attempts = 5;
				memset(buffer, 0, sizeof buffer);
				memset(full_message, 0, sizeof full_message);
				// read from current pointer in file to buffer
				// char* file_content = buffer;
				// strcpy(full_message, "0401");
				// strcpy(full_message + 3, package_nr);
				full_message[0] = 0;
				full_message[1] = 3;
				full_message[2] = package_nr >> 8;
				full_message[3] = package_nr;
				curr_file_size = fread(full_message + 4, 1, sizeof buffer, file);
				// strcpy(full_message + 4, file_content);
				package_nr++;

				printf("%d\n", full_message[1]);
				printf("%d\n", full_message[3]);
				printf("%s\n", full_message + 4);
				//for (int i = 4; i < 100; i++)
				//{
					//printf("%s", full_message[i]);
				//}
				//fprintf(stdout, "Full Message: %s\n", full_message);
				//fflush(stdout);
				// file is empty
				if (curr_file_size <= 0)
				{
					break_next = 1;
				}
				while (--retry_attempts)
				{
					ssize_t return_code = sendto(socket_file_descriptor, full_message, curr_file_size + 4, 0, (struct sockaddr *)&client, len);
					printf("Return Code: %d\n", return_code);
					if (return_code < 0)
					{
						printf("Error sending message");
					}
					ssize_t ack_return_code = recvfrom(socket_file_descriptor, message, strlen(message), 0, (struct sockaddr *)&client, &len);
					// printf("%d\n", ack_return_code);
					if (ack_return_code < 0)
					{
						printf("Error with sent message");
					}
					else
					{
						printf("Printing return opcode:");
						printf("%d\n", message[1]);
						break;
					}
				}

				if (break_next == 1)
				{
					break;
				}
				if (curr_file_size < 512)
				{
					//memset(full_message, 0, curr_file_size);
					//break_next = 1;
				}
			}
			printf("Done sending or smt");
		}
		else
		{
			printf("Ohhhhh, now you fucked uup!");
		}
		fflush(stdout);
	}

	return 0;
}
