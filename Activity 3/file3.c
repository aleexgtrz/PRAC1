#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>

#define DEFAULT_PORT 9999
#define BUFFER_SIZE 5
#define MAX_CHARS_PER_LINE 100

void err_sys(char *mess)
{
    perror(mess);
    exit(1);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in echoserver, echoclient;
    unsigned int echolen, clientlen, serverlen;
    char buffer[BUFFER_SIZE];
    int port, sock, result;
    char *file_name = "example.txt";
    int received = 0;

    /* Check input arguments */
    if (argc == 2)
    {
        file_name = argv[1];
        port = DEFAULT_PORT;
    }
    else if (argc == 3)
    {
        file_name = argv[1];
        port = atoi(argv[2]);
    }
    else
    {
        /* Wrong number of arguments */
        printf("Usage: %s [FILE_NAME] [PORT]\n", argv[0]);
        exit(1);
    }

    int line_count = 0;
    int lines[1024];
    char line[MAX_CHARS_PER_LINE];

    /* Open the file */
    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        err_sys("Error while opening file");
    }

    /* Read the file line by line */
    while (fgets(line, MAX_CHARS_PER_LINE, file) != NULL)
    {
        /* Saves the length of the line in the array*/
        lines[line_count] = strlen(line) % 100;
        line_count++;
    }
    fclose(file);

    /* Create UDP socket */
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        err_sys("Error socket");
    }

    /* Prepare sockaddr_in structure for server address */
    memset(&echoserver, 0, sizeof(echoserver));     /* Erase the memory area */
    echoserver.sin_family = AF_INET;                /* Internet/IP */
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY); /* Receive requests from any IP address valid on server */
    echoserver.sin_port = htons(port);     /* Server port */

    /* Get size of echoserver structure */
    serverlen = sizeof(echoserver);

    /* Bind that socket with the OS, to be able to receive messages on that socket */
    result = bind(sock, (struct sockaddr *)&echoserver, serverlen);
    if (result < 0)
    {
        err_sys("Error bind");
    }

    while (1)
    {
        /* Set the maximum size for address */
        clientlen = sizeof(echoclient);

        /* Receives first request */
        received = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&echoclient, &clientlen);
        if (received < 0)
        {
            err_sys("Error receiveing word from client");
        }

        /* Print client address */
        fprintf(stderr, "Client: %s, Message: %s\n", inet_ntoa(echoclient.sin_addr), buffer);

        /* Try to send number of lines*/
        sprintf(buffer, "%d", line_count);
        result = sendto(sock, buffer, received, 0, (struct sockaddr *)&echoclient, sizeof(echoclient));
        if (result != received)
        {
            err_sys("Error writing message back to the client");
        }

        /* Receives second request */
        received = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&echoclient, &clientlen);
        if (received < 0)
        {
            err_sys("Error receiveing word from client");
        }

        /* Print client address */
        fprintf(stderr, "Client: %s, Message: %s\n", inet_ntoa(echoclient.sin_addr), buffer);


        printf("Number: %d\n", lines[atoi(buffer)]);

        /* Try to send number of lines*/
        sprintf(buffer, "%d", lines[atoi(buffer)]);
        result = sendto(sock, buffer, received, 0, (struct sockaddr *)&echoclient, sizeof(echoclient));
        if (result != received)
        {
            err_sys("Error writing message back to the client");
        }
    }
}