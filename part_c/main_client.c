/**
 * @file main_client.c
 * @author Muhammet Sen (muhammet.sen@boun.edu.tr)
 * @brief Client code to demonstrate RPC between seperate computers.
 * Gets input from user, makes the request to RPC server and saves the result
 * in the given file.
 * @date 2021-12-13
 */
#include "main.h"
/**
 * @brief Take two numbers, path to an executable that will use those numbers, and server for RCP call.
 * 
 * @param host Server address for RPC
 * @param x Operand that will be sent to executable given in path
 * @param y Operand that will be sent to executable given in path
 * @param path Path of the executable 
 * @param outputFile Store outputs in a file
 */
void executer_1(char *host, int x, int y, char *path, char *outputFile)
{
	CLIENT *clnt; // RCP Client
	resultStruct *result_1; // Struct to 
	arguments execute_1_arg;

#ifndef DEBUG
	clnt = clnt_create(host, Executer, ADD_VERS, "udp");
	if (clnt == NULL)
	{
		clnt_pcreateerror(host);
		exit(1);
	}
#endif
	// Set parameters to according arguments that will be sent

	execute_1_arg.a = x;
	execute_1_arg.b = y;
	execute_1_arg.path = path;

	// Call the remote procedure
	result_1 = execute_1(&execute_1_arg, clnt);
	FILE *fptr = fopen(outputFile, "a");

	if (result_1->error)
	{
		fprintf(fptr, "FAIL:\n");
		fprintf(fptr,"%s", result_1->errorString);
	}
	else if (result_1 == (resultStruct *)NULL)
	{
		clnt_perror(clnt, "call failed");
	}
	else
	{
		fprintf(fptr, "SUCCESS:\n");
		fprintf(fptr, "%d\n", result_1->result);
	}
	fclose(fptr); // Close the file


#ifndef DEBUG
	clnt_destroy(clnt);
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
	int n1, n2;
	scanf("%d %d", &n1, &n2);

	// Get command line arguments
	char *host = argv[3];
	char *outputfile = argv[2];
	// make the call
	executer_1(host, n1, n2, argv[1], outputfile);

	exit(0);
}
