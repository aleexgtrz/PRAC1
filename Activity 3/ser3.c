#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>

#define BUFFER_SIZE 5
#define DEFAULT_PORT 8888
#define DEFAULT_IP "127.0.0.1"

void err_sys(char *mess)
{
  perror(mess);
  exit(1);
}

void handle_client(int sock, int number)
{
  char buffer[BUFFER_SIZE];
  int received = -1;
  int toreturn, n;

  printf("The random number is: %d\n", number);

  while (1)
  {
    /* Read from socket */
    n = read(sock, buffer, BUFFER_SIZE);
    if (n < 0)
      err_sys("Error reading from socket");
    /* Parse response from server */
    int received = atoi(buffer), toreturn = -1;

    printf("Correct: %d\n", number);
    printf("Received: %d\n", received);

    if (received == number)
    {
      toreturn = 0;
    }
    else if (received > number)
    {
      toreturn = 1;
    }
    else
    {
      toreturn = -1;
    }
    sprintf(buffer, "%d", toreturn);
    n = write(sock, buffer, BUFFER_SIZE);
    if (n < 0)
      err_sys("Error writing to socket");

    if (toreturn == 0)
    {
      return;
    }
  }
}

int main(int argc, char *argv[])
{
  struct sockaddr_in echoserver, echoclient;
  unsigned int echolen, clientlen, serverlen;
  char buffer[BUFFER_SIZE];
  int tcp_socket, udp_socket, client_socket, result, udp_port, tcp_port;
  int received = 0, random_number;
  char *server_ip;

  // Set values for IP and port number

  if (argc == 4)
  {
    // three arguments passed, use them as UDP port, ip_address and TCP port
    udp_port = atoi(argv[1]);
    server_ip = argv[2];
    tcp_port = atoi(argv[3]);
  }
  else
  {
    // wrong number of arguments
    printf("Usage: %s [UDP_PORT] [IP_ADDRESS] [TCP_PORT]\n", argv[0]);
    exit(1);
  }
  /* Create UDP socket */
  udp_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (udp_socket < 0)
  {
    err_sys("Error socket");
  }

  /* Configure/set socket address for the server (UDP) */
  memset(&echoserver, 0, sizeof(echoserver));        /* Erase the memory area */
  echoserver.sin_family = AF_INET;                   /* Internet/IP */
  echoserver.sin_addr.s_addr = inet_addr(server_ip); /* IP address */
  echoserver.sin_port = htons(udp_port);             /* Server port */

  /* Try to send the word to server */
  result = sendto(udp_socket, "-", 5, 0, (struct sockaddr *)&echoserver, sizeof(echoserver));
  if (result < 0)
  {
    err_sys("Error writing on socket");
  }

  /* Set the maximum size of address to be received */
  clientlen = sizeof(echoclient);

  /* Wait for echo from server */
  received = recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&echoclient, &clientlen);
  if (received < 0)
  {
    err_sys("Error reading");
  }

  /* Number of lines*/
  int lines = atoi(buffer);
  srand(time(NULL));
  sprintf(buffer, "%d", rand() % lines);

  result = sendto(udp_socket, buffer, 5, 0, (struct sockaddr *)&echoserver, sizeof(echoserver));
  if (result < 0)
  {
    err_sys("Error writing on socket");
  }

  /* Wait for echo from server */
  received = recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&echoclient, &clientlen);
  if (received < 0)
  {
    err_sys("Error reading");
  }

  /* Number of characters */
  random_number = atoi(buffer);
  printf("Number: %d\n", random_number);

  /* Create TCP socket */
  tcp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (tcp_socket < 0)
  {
    err_sys("Error socket");
  }

  /* Set information for sockaddr_in structure */
  memset(&echoserver, 0, sizeof(echoserver));     /* Reset memory */
  echoserver.sin_family = AF_INET;                /* Internet/IP */
  echoserver.sin_addr.s_addr = htonl(INADDR_ANY); /* Any address */
  echoserver.sin_port = htons(tcp_port);          /* Server port */

  /* Bind socket */
  result = bind(tcp_socket, (struct sockaddr *)&echoserver, sizeof(echoserver));
  if (result < 0)
  {
    err_sys("Error bind");
  }

  /* Listen socket */
  result = listen(tcp_socket, 1);
  if (result < 0)
  {
    err_sys("Error listen");
  }

  while (1)
  {
    unsigned int clientlen = sizeof(echoclient);

    /* Wait for client connection */
    client_socket = accept(tcp_socket, (struct sockaddr *)&echoclient, &clientlen);
    if (client_socket < 0)
    {
      err_sys("Error accept");
    }
    fprintf(stdout, "Client: %s\n", inet_ntoa(echoclient.sin_addr));

    /* Call function to handle socket*/
    handle_client(client_socket, random_number);
  }

  /* Close socket */
  close(udp_socket);
  close(tcp_socket);
  exit(0);
}