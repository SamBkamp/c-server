#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

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

void format_http_response(http_response *res, char* str, size_t limit){
  char* res_str = "\
HTTP/2 404\r\n\
Content-Type: text/html\r\n\
Content-Length: 21\r\n\
Connection: Closed\r\n\r\n\
<h1>Hello world</h1>\r\n";

  strncpy(str, res_str, limit);
}

int open_connection(uint16_t PORT, connection_info *ret){
  ret->sockfd = socket(AF_INET, SOCK_STREAM, 0); //open a socket
  if(ret->sockfd == -1){
    perror("socket error");
    return -1;
  }

  memset(&ret->my_addr, 0, sizeof(ret->my_addr)); //reset our socket address structs
  ret->my_addr.sin_family = AF_INET; //were using IP
  ret->my_addr.sin_port = htons(PORT); //turn from host byte order (little endian) to network byte (big endian)
  ret->my_addr.sin_addr = (struct in_addr){INADDR_ANY}; //accept connections from anywhere (0.0.0.0)
  //only accepts the in_addr struct which takes one value, for some reason.

  if(bind(ret->sockfd, (struct sockaddr *)&ret->my_addr, sizeof(ret->my_addr)) != 0){ //bind our socket to our address
    perror("bind error");
    return -1;
  }
  if(listen(ret->sockfd, 5) == -1){ //set our socket as listening
    perror("listen error");
    return -1;
  }
  return 0;
}

int main(int argc, char* argv[]){
  connection_info ci;
  char in_buf[1024];

  if(open_connection(8080, &ci) != 0){
    return 1;
  }

  socklen_t size_of_peer = sizeof(ci.peer_addr);
  int peer_socket = accept(ci.sockfd, (struct sockaddr *)&ci.my_addr, &size_of_peer);

  if(peer_socket == -1){
    perror("accept error");
    return 1;
  }

  http_response res = {0};
  res.code = 200;
  res.content_type = "text/html";

  ssize_t in_val = read(peer_socket, in_buf, 1023);
  printf("%s\n", in_buf);
  format_http_response(&res, in_buf, 1024);
  printf("%s\n", in_buf);
  send(peer_socket, in_buf, strlen(in_buf), 0);

}
