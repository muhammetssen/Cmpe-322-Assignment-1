/**
 * @file main_server.c
 * @author Muhammet Sen (muhammet.sen@boun.edu.tr)
 * @brief This program demonstrates how to establish communication between two processes using Remote Procedure Calls.
 * This server code creates a child process when a new request comes and use two pipes to communicate with the child.
 * After initialization, child's STDIN and STDOUT are connected to pipes mentioned before so that parent directly send and get data 
 * from a program designed for command line interactions like getting input and giving output.
 * @date 2021-12-13
 */

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define READ_END 0		// We will use 0 for read end
#define WRITE_END 1		// and 1 for write end
char message[100] = ""; // Message that will be sent to logger service

// Signature of the log_event function. Check below for further explaination
void log_event(char *host, int port);

resultStruct *
execute_1_svc(arguments *argp, struct svc_req *rqstp, int port, char *host)
{
	static resultStruct result; // Response of the blackbox and errors (if any) will be stored her

	// Pipes to communicate between parent and child
	int p_c[2], c_p_out[2], c_p_error[2];
	pid_t pid; // Process id of the child
	char errorBuffer[10000] = "";
	char outBuffer[10000] = "";

	pipe(p_c);		 // Pipe from parent to child
	pipe(c_p_error); // Pipe from child to parent
	pipe(c_p_out);	 // Pipe from child to parent

	if ((pid = fork()) == -1)
	{
		fprintf(stderr, "fork() failed.\n");
		exit(-1);
	}

	if (pid > 0)
	{
		// For the parent

		close(p_c[READ_END]);		 // Parent will not use the read end of p_c pipe since it should not read what it wrote.
		close(c_p_error[WRITE_END]); // Parent will not use the write end of c_p pipe since it will not send a message to itself.
		close(c_p_out[WRITE_END]);	 // Parent will not use the write end of c_p pipe since it will not send a message to itself.

		// Merge integers into one string
		sprintf(message, "%d %d", argp->a, argp->b);

		// Send integers to blackbox by using pipe p_c
		write(p_c[WRITE_END], message, (strlen(message) + 1));
		close(p_c[WRITE_END]);

		int errBytes = read(c_p_error[READ_END], errorBuffer, sizeof(errorBuffer)); // Read Error Pipe
		int outBytes = read(c_p_out[READ_END], outBuffer, sizeof(outBuffer));		// Read Output Pipe
		// Closes pipes since we are not expecting any more message from child
		close(c_p_error[READ_END]);
		close(c_p_out[READ_END]);

		// If something is in the error pipe, errBytes will be > 0
		if (errBytes > 0)
		{
			// result.error boolean is set to 1 to warn client
			result.error = 1;
			result.errorString = errorBuffer; // Save the error message

			sprintf(message, "%s _\n", message); // Add _ to log
			log_event(host, port);				 // Send the log the log server
		}
		else
		{
			result.error = 0;				 // Child did not return any errors
			result.errorString = "";		 // Empty error string
			result.result = atoi(outBuffer); // Convert char* to integer and save to result struct

			sprintf(message, "%s %d\n", message, result.result); // Add the output to log
			log_event(host, port);								 // Send the log the log server
		}
	}
	else
	{
		// For the child

		dup2(p_c[READ_END], STDIN_FILENO);		   // Connect pipe's read end to standart input so we can use parent's messages in scanf function.
		dup2(c_p_out[WRITE_END], STDOUT_FILENO);   //Similarly, we can send printf function's output to parent via pipe.
		dup2(c_p_error[WRITE_END], STDERR_FILENO); //And send errors.

		// Close pipes since we will not use them directly anymore
		close(c_p_error[READ_END]);
		close(c_p_error[WRITE_END]);
		close(c_p_out[READ_END]);
		close(c_p_out[WRITE_END]);
		close(p_c[WRITE_END]);
		close(p_c[READ_END]);

		// Run blackbox
		execl(argp->path, argp->path, NULL); // Execute the "blackbox"
	}

	return &result;
}
/**
 * 
// This code is mainly taken from http://www.qnx.com/developers/docs/qnx_4.25_docs/tcpip50/prog_guide/sock_ipc_tut.html
 * @brief Send `message` variable's content to log server
 * 
 * @param host IP address of the log server
 * @param port Port of the log server
 */
void log_event(char *host, int port)
{

	int sock;
	struct sockaddr_in server;
	struct hostent *hp, *gethostbyname();

	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("opening stream socket");
		exit(1);
	}
	// Connect created socket to specified host
	server.sin_family = AF_INET;
	hp = gethostbyname(host);
	if (hp == 0)
	{
		// Could not find corresponding host
		fprintf(stderr, "%s: unknown host\n", host);
		exit(2);
	}
	memcpy(&server.sin_addr, hp->h_addr, hp->h_length); // Copy host to server struct
	server.sin_port = htons(port);						// Specify which port to use
	if (connect(sock, (struct sockaddr *)&server,
				sizeof(server)) < 0)
	{
		perror("connecting stream socket");
		exit(1);
	}
	if (write(sock, message, sizeof(message)) < 0) // Send the log to server
		perror("writing on stream socket");
	close(sock);
}
