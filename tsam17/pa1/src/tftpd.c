/* your code goes here. */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define RRQ   1
#define WRQ   2
#define DATA  3
#define ACK   4
#define ERROR 5

void print_client_origin(struct sockaddr_in client, char* file_name)
{
	int client_port = ntohs(client.sin_port);
	unsigned char *client_ip = (unsigned char *)&client.sin_addr.s_addr;

	printf("file name \"%s", file_name);
	printf("\" requested from ");
	printf("%d.%d.%d.%d:", client_ip[0], client_ip[1], client_ip[2], client_ip[3]);
	printf("%d\n", client_port);
}

void send_error_to_client(int error_value, int socket_file_descriptor, struct sockaddr_in client, socklen_t len, char* message)
{
	char full_message[512];
	memset(full_message, 0, sizeof full_message);
	full_message[0] = 0;
	full_message[1] = 5;
	full_message[2] = 0;
	full_message[3] = error_value;
	strcpy(full_message + 4, message);
	sendto(socket_file_descriptor, full_message, 512, 0, (struct sockaddr *)&client, len);
}

int main(int argc, char *argv[])
{
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
	server.sin_port = htons(atoi(argv[argc - 2]));
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

		if (n < 0)
		{
			printf("Something went wrong\n");
		}

		// the second byte in the array contains the Opcode
		if (message[1] == RRQ)
		{
			// We jump to over the first two bytes to get the filename
			char *file_name = message + 2;
			// Length of given file name
			size_t f_n_length = strlen(file_name);

			// get client port and ip, print alongside file name requested
			print_client_origin(client, file_name);

			size_t argv_length = strlen(argv[2]);

			// Initialize array for data path
			char full_path[argv_length + f_n_length + 2];

			// construct the full path from client message
			strcpy(full_path, argv[2]);
			strcpy(full_path + argv_length, "/");
			strcpy(full_path + argv_length + 1, file_name);

			FILE *file;
			
			// if name from client contains a slash, deny that access
			if (strchr(file_name, '/') != NULL)
			{
				send_error_to_client(2, socket_file_descriptor, client, len, "Access violation.\0");
				file = NULL;
			}
			else
			{
				// open file from location
				file = fopen(full_path, "r");
			}

			if (file == NULL)
			{
				send_error_to_client(1, socket_file_descriptor, client, len, "File not found.\0");
			}
			else
			{
				// Keep track of length of file
				fseek(file, 0, SEEK_END);
				// Go back to the beginning of the file
				fseek(file, 0, SEEK_SET);

				int curr_file_size;
				char full_message[516];
				int break_next = 0;
				unsigned short sent_block_nr = 1;
				while (1)
				{
					int retry_attempts = 5;
					memset(full_message, 0, sizeof full_message);
					memset(message, 0, sizeof message);
					// construct data pack that will be sent to client
					full_message[0] = 0;
					full_message[1] = 3;
					full_message[2] = sent_block_nr >> 8;
					full_message[3] = sent_block_nr;
					curr_file_size = fread(full_message + 4, 1, 512, file);

					// if file is (near) empty, then last message is sent
					if (curr_file_size < 512)
					{
						break_next = 1;
					}
					// send to and receive from client
					while (--retry_attempts)
					{
						ssize_t return_code = sendto(socket_file_descriptor, full_message, curr_file_size + 4, 0, (struct sockaddr *)&client, len);

						if (return_code < 0)
						{
							printf("Error sending message");
						}
						ssize_t ack_return_code = recvfrom(socket_file_descriptor, &message, sizeof(message), 0, (struct sockaddr *)&client, &len);
						unsigned short message_1 = message[2];
						unsigned short message_2 = message[3] & 0xFF;
						unsigned short received_block_nr = (message_1 << 8) | message_2;

						if (ack_return_code < 0)
						{
							printf("Error with sent message");
						}
						else if (message[1] == ACK && received_block_nr == sent_block_nr)
						{
							// nothing went wrong so pack was successfully sent
							sent_block_nr++;
							break;
						}
					}

					if (break_next == 1 || retry_attempts == 0)
					{
						break;
					}
				}
			}

		}
		// Handle write request denial
		else if (message[1] == WRQ)
		{
			printf("Write requested, denied!\n");
			send_error_to_client(2, socket_file_descriptor, client, len, "Access violation.\0");

		}
		fflush(stdout);
	}

	return 0;
}
