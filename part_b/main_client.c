
/**
 * @file main_client.c
 * @author Muhammet Sen (muhammet.sen@boun.edu.tr)
 * @brief Client code to demonstrate RPC between seperate computers.
 * Gets input from user, makes the request to RPC server and saves the result
 * in the given file.
 * @date 2021-12-07
 */

#include "main.h"
#define MAX_INTEGER_LENGTH 12 // Maximum value of integer data type is 2**31 -1 = 2147483648. So need a larger buffer to store that number.

/**
 * @brief Take two numbers, path to an executable that will use those numbers, and server for RCP call.
 * 
 * @param host Server address for RPC
 * @param x Operand that will be sent to executable given in path
 * @param y Operand that will be sent to executable given in path
 * @param path Path of the executable 
 * @param result Pointer to store the result
 */
void executer(char *host, int x, int y, char *path, int *result)
{
	CLIENT *clnt;			 // RCP Client
	int *result_1;			 // Result will be stored in this pointer's address
	arguments execute_1_arg; // Arguments to sent to RCP Server

#ifndef DEBUG
	clnt = clnt_create(host, ADD_PROG, ADD_VERS, "udp"); // Create a client using UDP (User Datagram Protocol)
	if (clnt == NULL)
	{
		clnt_pcreateerror(host);
		exit(1); // Exit if cannot create a client
	}
#endif
	// Set parameters to according arguments that will be sent
	execute_1_arg.a = x;
	execute_1_arg.b = y;
	execute_1_arg.path = path;

	// Call the remote procedure
	result_1 = execute_1(&execute_1_arg, clnt);
	if (result_1 == (int *)NULL) // There may be a problem since server did not respond with the valid integer.
	{
		clnt_perror(clnt, "call failed");
	}
	else
	{
		*result = *result_1; // Save the result returned by the server to result parameter given
	}
#ifndef DEBUG
	clnt_destroy(clnt); // Close the connection by destroying the client
#endif
}

int main(int argc, char *argv[])
{

	if (argc < 4)
	{
		printf("usage: %s executable output_file server_host\n", argv[0]);
		exit(1);
	}
	
	// Read two integers from command line
	int n1,n2;
	scanf("%d %d", &n1, &n2);
	
	// Get command line arguments
	char *host = argv[3];
	char *outputfile = argv[2];
	// Create a variable to store the final result
	int result = 0;
	// make the call
	executer(host, n1, n2, argv[1], &result);
	
	// Open the given file in 'append' mode
	FILE *fptr = fopen("output.txt", "a");
	// Append result to file
	fprintf(fptr, "%d\n", result);
	// Close the file after writing
	fclose(fptr);
	exit(0);
}