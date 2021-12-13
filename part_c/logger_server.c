
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define TRUE 1

int main(int argc, char *argv[])
{
    if (argc < 2)

    {
        printf("usage: %s output_file port\n", argv[0]);
        exit(1);
    }

    char *log_file = argv[1];
    int port_number = atoi(argv[2]);

    int sock, length;
    struct sockaddr_in server;
    int msgsock;
    char buf[1024];
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
    // printf("Socket has port #%d\n", ntohs(server.sin_port));

    /* Start accepting connections */

    listen(sock, 5);
    do
    {
        msgsock = accept(sock, 0, 0);
        if (msgsock == -1)
        {
            perror("accept");
            return EXIT_FAILURE;
        }
        else
        {
            // Starting a connection
            do //read while connection is open
            {
                memset(buf, 0, sizeof(buf));
                if ((rval = read(msgsock, buf, 1024)) < 0)
                    perror("reading stream message");
                else if (rval == 0)
                {
                    // Ending connection
                }
                else
                {
                    FILE *fptr = fopen(log_file, "a");
                    printf("Incoming Log: %s\n",buf);
                    fprintf(fptr, "%s\n", buf);
                    fclose(fptr);
                }
            } while (rval > 0);
        }
        close(msgsock);
    } while (TRUE);
}