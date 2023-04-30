#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define MAX_PENDING 1
#define BUFFSIZE 5
#define DEFAULT_PORT 8888

// Prints an error message and exits the program
void err_sys(const char *error_msg)
{
    perror(error_msg);
    exit(1);
}

// Reads data from the socket and returns it as an integer value
int readFromSocket(int sock, char *buffer)
{
    int n = read(sock, buffer, BUFFSIZE);
    if (n < 0)
    {
        err_sys("Error reading from socket");
    }
    return atoi(buffer); 
}

// Writes an integer value to the socket
void writeToSocket(int sock, int value, char *buffer)
{  
    sprintf(buffer, "%d", value);
    int n = write(sock, buffer, BUFFSIZE);
    if (n < 0)
    {
        err_sys("Error writing to socket");
    }
}

// Determines the return value based on the received value and the correct number.
int getReturnValue(int received, int random_number)
{
    if (received == random_number)
        return 0; // Value matches the random number
    else if (received > random_number)
        return 1; // Value is greater than the random number
    else
        return -1; // Value is smaller than the random number
}

// Handles clients connection. Reads the clients guess and returns a number to orientate it. 
void handleClient(int sock)
{
    char buffer[BUFFSIZE];
    int received, random_number, toreturn;

    // Seeds the random number generator
    srand(time(NULL));

    // Generates a random number between 0 and 100
    random_number = rand() % 101;
    printf("The random number is: %d\n", random_number);

    while (1)
    {
        received = readFromSocket(sock, buffer); 
        toreturn = getReturnValue(received, random_number); 
        writeToSocket(sock, toreturn, buffer); 

        // Value matches guessed number.
        if (toreturn == 0)
            break;
    }
    close(sock);
}

// Creates a socket and sets it up for listening
int createSocket(int port)
{   
    // Creates a TCP socket
    int server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock < 0)
        err_sys("Error creating socket");

    struct sockaddr_in echoserver;
    memset(&echoserver, 0, sizeof(echoserver));
    echoserver.sin_family = AF_INET;
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY);
    echoserver.sin_port = htons(port);

    // Binds the socket to the specified address and port
    int result = bind(server_sock, (struct sockaddr *)&echoserver, sizeof(echoserver)); 
    if (result < 0)
        err_sys("Error binding socket");

    // Listens for incoming clients connections
    result = listen(server_sock, MAX_PENDING); 
    if (result < 0)
        err_sys("Error listening socket");

    return server_sock;
}

// Wait for and handle client connections
void wait(int server_sock)
{
    struct sockaddr_in echoclient;
    while (1)
    {
        unsigned int clientlen = sizeof(echoclient);
        int client_sock = accept(server_sock, (struct sockaddr *)&echoclient, &clientlen);
        if (client_sock < 0)
            err_sys("Error accepting client");

        printf("Client: %s\n", inet_ntoa(echoclient.sin_addr));

        handleClient(client_sock);
    }
}

int main(int argc, char *argv[])
{
    int port = DEFAULT_PORT;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    else if(argc > 2)
    {
        err_sys("Usage: ser1 [port]");
    }
    int server_sock = createSocket(port);
    wait(server_sock);

    exit(0);
}