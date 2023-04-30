#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define MAX_PENDING 1
#define BUFFSIZE 5
#define DEFAULT_PORT 8888
#define MAX_CHARS_PER_LINE 100

/* Error handler*/
void err_sys(char *mess)
{
  perror(mess);
  exit(1);
}

int lines[1024];
int line_count = 0;

int generateRandom()
{
  srand(time(NULL));
  return lines[rand() % line_count];
}

void handle_client(int sock)
{
  char buffer[BUFFSIZE];
  int received = -1;
  int toreturn, n;

  int random_number = generateRandom();
  printf("The random number is: %d\n", random_number);

  while (1)
  {
    /* Read from socket */
    n = read(sock, buffer, BUFFSIZE);
    if (n < 0)
      err_sys("Error reading from socket");
    /* Parse response from server */
    int received = atoi(buffer), toreturn = -1;

    printf("Correct: %d\n", random_number);
    printf("Received: %d\n", received);

    if (received == random_number)
    {
      toreturn = 0;
    }
    else if (received > random_number)
    {
      toreturn = 1;
    }
    else
    {
      toreturn = -1;
    }

    sprintf(buffer, "%d", toreturn);
    n = write(sock, buffer, BUFFSIZE);
    if (n < 0)
      err_sys("Error writing to socket");

    if (toreturn == 0)
    {
      break;
    }
  }
  /* Close socket */
  close(sock);
}

int createServerSocket()
{
  /* Create TCP socket */
  int server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_sock < 0)
  {
    err_sys("Error socket");
  }

  return server_sock;
}

void set_sockaddr_in(struct sockaddr_in *echoserver, int port)
{
  memset(echoserver, 0, sizeof(*echoserver));      /* Reset memory */
  echoserver->sin_family = AF_INET;                /* Internet/IP */
  echoserver->sin_addr.s_addr = htonl(INADDR_ANY); /* Any address */
  echoserver->sin_port = htons(port);              /* Server port */
}

void readFile(char *file_name)
{
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
}

int main(int argc, char *argv[])
{
  struct sockaddr_in echoserver, echoclient;
  int client_sock, port;
  char *file_name = "file.txt";

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

  int server_sock = createServerSocket();
  set_sockaddr_in(&echoserver, port);

  /* Bind socket */
  int result = bind(server_sock, (struct sockaddr *)&echoserver, sizeof(echoserver));
  if (result < 0)
  {
    err_sys("Error bind");
  }

  /* Listen socket */
  result = listen(server_sock, MAX_PENDING);
  if (result < 0)
  {
    err_sys("Error listen");
  }

  readFile(file_name);

  while (1)
  {
    unsigned int clientlen = sizeof(echoclient);

    /* Wait for client connection */
    client_sock = accept(server_sock, (struct sockaddr *)&echoclient, &clientlen);
    if (client_sock < 0)
    {
      err_sys("Error accept");
    }
    fprintf(stdout, "Client: %s\n", inet_ntoa(echoclient.sin_addr));

    /* Call function to handle socket*/
    handle_client(client_sock);
  }
  close(server_sock);
  exit(1);
}