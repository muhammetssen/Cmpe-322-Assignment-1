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

#define READ_END 0			  // We will use 0 for read end
#define WRITE_END 1			  // and 1 for write end

resultStruct *
execute_1_svc(arguments *argp, struct svc_req *rqstp)
{
	static resultStruct result;

	// Pipes to communicate between parent child
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

		char message[10000] = "";
		sprintf(message, "%d %d", argp->a, argp->b);
		// strcat(number_1, "\n");

		write(p_c[WRITE_END], message, (strlen(message) + 1));
		close(p_c[WRITE_END]);

		int errBytes = read(c_p_error[READ_END], errorBuffer, sizeof(errorBuffer));
		int outBytes = read(c_p_out[READ_END], outBuffer, sizeof(outBuffer));
		close(c_p_error[READ_END]);
		close(c_p_out[READ_END]);
		if (errBytes > 0)
		{
			// result.error boolean is set to 1 to warn client
			result.error = 1;
			result.errorString = errorBuffer; // Save the error message
		}
		else
		{
			result.error = 0; // Child did not return any errors
			result.errorString = ""; // Empty error string
			result.result = atoi(outBuffer); // Convert char* to integer and save to result struct
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
