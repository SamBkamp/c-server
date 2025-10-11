#ifndef SERVER_PROT
#define SERVER_PROT

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
  uint16_t code;
  char* content_type;
}http_response;

#endif
