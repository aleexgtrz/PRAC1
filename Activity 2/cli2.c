#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define DEFAULT_PORT 8888
#define DEFAULT_IP "127.0.0.1"
#define BUFFSIZE 5

// Prints an error message and exits the program
void err_sys(const char *error_msg)
{
    perror(error_msg);
    exit(1);
}

// Connects to the server with the given IP address and port number
int connectToServer(int port, const char *ip_addr)
{
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip_addr);
    server_address.sin_port = htons(port);

    // Create a socket for communication with the server
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {
        err_sys("Error creating socket");
    }

    // Connect to the server using the socket
    int result = connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));
    if (result < 0)
    {
        err_sys("Error connecting to server");
    }

    printf("Connected to server\n");

    return sock;
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

// Plays the number guessing game with the server
void play(int sock)
{
    int guess, min = 0, max = 100, iterations = 0;
    char buffer[BUFFSIZE];

    while (1)
    {
        guess = (max + min) / 2;
        printf("Guessing %d\n", guess);

        // Send the guess to the server
        writeToSocket(sock, guess, buffer);

        int result = readFromSocket(sock, buffer);

        if (result == 0)
        {
            printf("Correct guess: %d\n", guess);
            break;
        }
        else if (result > 0)
        {
            printf("Guess is too high\n");
            max = guess;
        }
        else if (result < 0)
        {
            printf("Guess is too low\n");
            min = guess;
        }
        else
        {
            err_sys("Error parsing response from server");
        }
        iterations++;
    }
    printf("Number of iterations: %d\n", iterations);
}

int main(int argc, char *argv[])
{
    int port, sock;
    const char *ip_addr;

    // Parse the command line arguments
    if (argc == 1)
    {
        port = DEFAULT_PORT;
        ip_addr = DEFAULT_IP;
    }
    else if (argc == 2)
    {
        port = atoi(argv[1]);
        ip_addr = DEFAULT_IP;
    }
    else if (argc == 3)
    {
        port = atoi(argv[1]);
        ip_addr = argv[2];
    }
    else
    {
        err_sys("Usage: cli2 [port] [ip]");
    }

    sock = connectToServer(port, ip_addr);

    play(sock);

    close(sock);
    exit(0);
}