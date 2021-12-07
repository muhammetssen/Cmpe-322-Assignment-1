/**
 * @file main.c
 * @author Muhammet Sen (you@domain.com)
 * @date 2021-12-07
 * #TODO Write @brief
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
    if(argc < 2){
        printf ("usage: %s executable result_file\n", argv[0]);
		exit (1);
        return -1;
    }

    char * executablePath= argv[1];
    char * outputFilePath= argv[2];
    
    // Pipes to communicate between parent child
    int p_c[2], c_p_out[2], c_p_error[2];
    pid_t pid; // Process id of the child
    char errorBuffer[10000];
    char outBuffer[10000];
 
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

        char number_1[MAX_INTEGER_LENGTH], number_2[MAX_INTEGER_LENGTH];
        scanf("%s %s", number_1, number_2);

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
        // printf("PARENT: Here is the response from the child -> %s \n", buffer);
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

        // int a, b;
        execl(executablePath, NULL);
        // scanf("%d %d", &a, &b);
        // printf("CHILD:\tnumber1 = %s \n", a);
        // printf("CHILD:\tnumber2 = %s \n", b);
        // printf("%d", a+b);
        // int number1, number2;

        // read(p_c[READ_END], buffer, sizeof(buffer));
        // number1 = atoi(buffer);

        // read(p_c[READ_END], buffer2, sizeof(buffer2));
        // number2 = atoi(buffer2);

        // close(p_c[READ_END]);
        // int result = number1 + number2;
        // printf("number1 = %d \n", number1);
        // printf("number2 = %d \n", number2);
        // printf("result = %d \n", result);

        // char resultChar[MAX_INTEGER_LENGTH];

        // sprintf(resultChar, "%d", result);

        // write(c_p[WRITE_END], resultChar, (strlen(resultChar) + 1));
        // close(c_p[WRITE_END]);
    }

    return 0;
}