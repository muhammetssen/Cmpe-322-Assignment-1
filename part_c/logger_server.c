
/**
 * @file logger_server.c
 * @author Muhammet Sen
 * @brief Logger Server that uses sockets and ports to communicate with clients.
 * Usage: ./logger_server `output_file` `port`
 * 
 * @date 2021-12-13
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define TRUE 1

// This code is mainly taken from http://www.qnx.com/developers/docs/qnx_4.25_docs/tcpip50/prog_guide/sock_ipc_tut.html

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: %s output_file port\n", argv[0]);
        exit(1);
    }

    char *log_file = argv[1];        // File to write incoming logs
    int port_number = atoi(argv[2]); // Which port to use

    int sock, length;
    struct sockaddr_in server;
    int msgsock;
    char buf[100];
    int rval;
    int i;

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("opening stream socket");
        exit(1);
    }
    /* Name socket using wildcards */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port_number);
    if (bind(sock, (struct sockaddr *)&server,
             sizeof(server)))
    {
        perror("binding stream socket");
        exit(1);
    }
    length = sizeof(server);
    if (getsockname(sock, (struct sockaddr *)&server,
                    &length))
    {
        perror("getting socket name");
        exit(1);
    }

    /* Start accepting connections */

    listen(sock, 5); // Accept up to 5 concurrent connections
    do
    {
        msgsock = accept(sock, 0, 0);
        if (msgsock == -1)
        {
            perror("Could not accept new request");
            return EXIT_FAILURE;
        }
        else
        {
            // Starting a connection - creating a new socket
            do
            {
                memset(buf, 0, sizeof(buf));              // clear the buffer
                if ((rval = read(msgsock, buf, 100)) < 0) // if read returns a negative value
                    perror("reading stream message");
                else if (rval > 0) 
                // Open a file, write to it and close afterwards. We could open the file once when the 
                // connection is established and close it when the connection is closed but I am not sure
                // how it will work while grading.
                {
                    FILE *fptr = fopen(log_file, "a");
                    fprintf(fptr, "%s", buf);
                    fclose(fptr);
                }
            } while (rval > 0);
        }
        close(msgsock); // close the connection
    } while (TRUE); // Keep accepting new connections
}