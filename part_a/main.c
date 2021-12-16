/**
 * @file main.c
 * @author Muhammet Sen (you@domain.com)
 * @date 2021-12-07
 * @brief Simply redirects pipes to child's input, output, and error.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define READ_END 0  // We will use 0 for read end
#define WRITE_END 1 // and 1 for write end
#define MAX_INTEGER_LENGTH 12
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: %s executable result_file\n", argv[0]);
        exit(1);
        return -1;
    }

    char *executablePath = argv[1];
    char *outputFilePath = argv[2];

    // Pipes to communicate between parent child
    int p_c[2], c_p_out[2], c_p_error[2];
    pid_t pid; // Process id of the child
    char errorBuffer[10000] = "";
    char outBuffer[10000] = "";

    pipe(p_c);       // Pipe from parent to child
    pipe(c_p_error); // Pipe from child to parent
    pipe(c_p_out);   // Pipe from child to parent

    if ((pid = fork()) == -1)
    {
        fprintf(stderr, "fork() failed.\n");
        exit(-1);
    }

    if (pid > 0)
    {
        // For the parent

        close(p_c[READ_END]);        // Parent will not use the read end of p_c pipe since it should not read what it wrote.
        close(c_p_error[WRITE_END]); // Parent will not use the write end of c_p pipe since it will not send a message to itself.
        close(c_p_out[WRITE_END]);   // Parent will not use the write end of c_p pipe since it will not send a message to itself.

        int number_1 = 0, number_2 = 0;
        scanf("%d %d", &number_1, &number_2);

        char message[10000] = "";
        sprintf(message, "%d %d", number_1, number_2);

        write(p_c[WRITE_END], message, (strlen(message) + 1));
        close(p_c[WRITE_END]);

        int errBytes = read(c_p_error[READ_END], errorBuffer, sizeof(errorBuffer));
        int outBytes = read(c_p_out[READ_END], outBuffer, sizeof(outBuffer));
        close(c_p_error[READ_END]);
        close(c_p_out[READ_END]);

        FILE *fptr = fopen(outputFilePath, "a");

        if (errBytes > 0)
        {
            fprintf(fptr, "FAIL:\n");
            fprintf(fptr, errorBuffer);
        }
        else
        {
            fprintf(fptr, "SUCCESS:\n");
            fprintf(fptr, outBuffer);
        }
        fclose(fptr);
    }
    else
    {
        // For the child

        dup2(p_c[READ_END], STDIN_FILENO);
        dup2(c_p_out[WRITE_END], STDOUT_FILENO);
        dup2(c_p_error[WRITE_END], STDERR_FILENO);

        close(c_p_error[READ_END]);
        close(c_p_error[WRITE_END]);
        close(c_p_out[READ_END]);
        close(c_p_out[WRITE_END]);
        close(p_c[WRITE_END]);
        close(p_c[READ_END]);

        execl(executablePath, NULL);
    }

    return 0;
}