#ifndef SERVER_PROT
#define SERVER_PROT

#include <stdint.h>
#include <netinet/in.h>


#ifndef VAL_MAX
#define VAL_MAX 256
#endif //VAL_MAX

typedef struct{
  char key[VAL_MAX];
  char value[VAL_MAX];
}kv_pair;

typedef struct{
  char endpoint[20];
  char symbol[10];
  unsigned long timestamp;
  kv_pair *data;
}quote_cache;

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
