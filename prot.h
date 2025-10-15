#ifndef SERVER_PROT
#define SERVER_PROT

#include <stdint.h>
#include <netinet/in.h>

typedef struct{
  int sockfd;
  struct sockaddr_in my_addr;
  struct sockaddr_in peer_addr;
}connection_info;

typedef struct{
  uint8_t method;
  char* host;
}http_connection;

typedef struct{
  int response_code;
  int content_length;
  char* body;
}http_response;

#endif
