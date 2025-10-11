#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "prot.h"
#include "conn.h"


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
