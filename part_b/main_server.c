
/**
 * @file main_server.c
 * @author Muhammet Sen (muhammet.sen@boun.edu.tr)
 * @brief This program demonstrates how to establish communication between two processes using Remote Procedure Calls.
 * This server code creates a child process when a new request comes and use two pipes to communicate with the child.
 * After initialization, child's STDIN and STDOUT are connected to pipes mentioned before so that parent directly send and get data 
 * from a program designed for command line interactions like getting input and giving output.
 * @date 2021-12-07 */

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define READ_END 0			  // We will use 0 for read end
#define WRITE_END 1			  // and 1 for write end
#define MAX_INTEGER_LENGTH 12 // Maximum value of integer data type is 2**31 -1 = 2147483648. So need a larger buffer to store that number.

resultStruct *
execute_1_svc(arguments *argp, struct svc_req *rqstp)
{
	static resultStruct result;

	// Pipes to communicate between parent child
	int p_c[2], c_p_out[2], c_p_error[2];
	pid_t pid; // Process id of the child
	char errorBuffer[10000];
	char outBuffer[10000];

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

		char number_1[MAX_INTEGER_LENGTH], number_2[MAX_INTEGER_LENGTH];
		sprintf(number_1, "%d", argp->a);
		sprintf(number_2, "%d", argp->b);

		// Merge Strings
		strcat(number_1, " ");
		strcat(number_1, number_2);
		strcat(number_1, "\n");

		write(p_c[WRITE_END], number_1, (strlen(number_1) + 1));
		close(p_c[WRITE_END]);

		int errBytes = read(c_p_error[READ_END], errorBuffer, sizeof(errorBuffer));
		int outBytes = read(c_p_out[READ_END], outBuffer, sizeof(outBuffer));
		close(c_p_error[READ_END]);
		close(c_p_out[READ_END]);
		if (errBytes > 0)
		{
			result.error = 1;
			result.errorString = errorBuffer;
			//errorbuffer
		}
		else
		{
			result.error = 0;
			result.errorString = "";
			result.result = atoi(outBuffer);
		}
	}
	else
	{
		// For the child

		dup2(p_c[READ_END], STDIN_FILENO);
		dup2(c_p_out[WRITE_END], STDOUT_FILENO);
		dup2(c_p_error[WRITE_END], STDERR_FILENO);
		// After duplicating, we will not use any more pipe operations so we can disable both ways for both tunnels.

		close(c_p_error[READ_END]);
		close(c_p_error[WRITE_END]);
		close(c_p_out[READ_END]);
		close(c_p_out[WRITE_END]);
		close(p_c[WRITE_END]);
		close(p_c[READ_END]);

		execl(argp->path, argp->path, NULL); // Execute the "blackbox"
	}

	return &result;
}
