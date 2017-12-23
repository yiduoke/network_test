#include "pipe_networking.h"

/*=========================
  server_setup
  args:

  creates the WKP (upstream) and opens it, waiting for a
  connection.

  removes the WKP once a connection has been made

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_setup() {
  remove("luigi");
  mkfifo("luigi", 0777);
  
  int from_client = open("luigi", O_RDONLY);
  
  remove("luigi");
  printf("setup done\n");
  
  return from_client;
}


/*=========================
  server_connect
  args: int from_client

  handles the subserver portion of the 3 way handshake

  returns the file descriptor for the downstream pipe.
  =========================*/
int server_connect(int from_client) {
  char buffer[BUFFER_SIZE];
  read(from_client, buffer, sizeof(buffer));
  printf("server read: %s\n", buffer);
  
  int to_client = open(buffer, O_WRONLY);
  write(to_client, ACK, sizeof(buffer));
  
  read(from_client, buffer, sizeof(buffer));
  printf("received ack: %s\n", buffer);
  printf("connect done\n");
  return to_client;
}

/*=========================
  server_handshake
  args: int * to_client

  Performs the server side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  
  int from_client;

  char buffer[HANDSHAKE_BUFFER_SIZE];
  
  mkfifo("luigi", 0600);
  
  //block on open, recieve mesage
  printf("[server] handshake: making wkp\n");
  from_client = open( "luigi", O_RDONLY, 0);
  read(from_client, buffer, sizeof(buffer));
  printf("[server] handshake: received [%s]\n", buffer);

  remove("luigi");
  printf("[server] handshake: removed wkp\n");
    
  //connect to client, send message
  *to_client = open(buffer, O_WRONLY, 0);
  write(*to_client, buffer, sizeof(buffer));

  //read for client
  read(from_client, buffer, sizeof(buffer));
  printf("[server] handshake received: %s\n", buffer);

  return from_client;
}

/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.
  
  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {

  int from_server;
  char buffer[BUFFER_SIZE];

  //send pp name to server
  printf("[client] handshake: connecting to wkp\n");
  *to_server = open( "luigi", O_WRONLY);
  if ( *to_server == -1 )
    exit(1);

  //make private pipe
  sprintf(buffer, "%d", getpid() );
  mkfifo(buffer, 0600);
  
  write(*to_server, buffer, sizeof(buffer));
  
  //open and wait for connection
  from_server = open(buffer, O_RDONLY, 0);
  read(from_server, buffer, sizeof(buffer));
  /*validate buffer code goes here */
  printf("[client] handshake: received [%s]\n", buffer);

  //remove pp
  remove(buffer);
  printf("[client] handshake: removed pp\n");

  //send ACK to server
  write(*to_server, ACK, sizeof(buffer));

  return from_server;
}
