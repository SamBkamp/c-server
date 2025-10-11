#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8080

int main(int argc, char* argv[]){
  struct sockaddr_in my_addr, peer_addr; //define the two socket addresses7

  int s = socket(AF_INET, SOCK_STREAM, 0); //open a socket
  if(s == -1){
    perror("socket error");
    return 1;
  }

  memset(&my_addr, 0, sizeof(my_addr)); //reset our socket address structs
  my_addr.sin_family = AF_INET; //were using IP
  my_addr.sin_port = htons(PORT); //turn from host byte order (little endian) to network byte (big endian)
  my_addr.sin_addr = (struct in_addr){INADDR_ANY}; //accept connections from anywhere (0.0.0.0)
  //only accepts the in_addr struct which takes one value, for some reason.


  if(bind(s, (struct sockaddr *)&my_addr, sizeof(my_addr)) != 0){ //bind our socket to our address
    perror("bind error");
    return 1;
  }
  if(listen(s, 5) == -1){ //set our socket as listening
    perror("listen error");
    return 1;
  }

  socklen_t size_of_peer = sizeof(peer_addr);
  int a = accept(s, (struct sockaddr *)&my_addr, &size_of_peer);

  if(a == -1){
    perror("accept error");
    return 1;
  }
}
