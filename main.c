#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "prot.h"
#include "conn.h"
#include "parse.h"

#define PORT 8080
#define SECONDS_PER_HOUR 3600
#define HK_OFFSET 28800 //8 hours


int main(){
  http_response res = {0};
  connection_info ci;
  char in_buf[1024]; //buffer for inbound connections
  char response_buff[2048]; //response from outbound connection
  kv_pair pairs[20];

  request_stock_data(response_buff, 2048);
  parse_http_response(&res, response_buff);
  json_parse(res.body, pairs);
  free(res.body);

  if(open_connection(PORT, &ci) != 0){
    return 1;
  }

  printf("Started server on port %d\n", PORT);

  socklen_t size_of_peer = sizeof(ci.peer_addr);
  while(1){
    int peer_socket = accept(ci.sockfd, (struct sockaddr *)&ci.my_addr, &size_of_peer);

    if(peer_socket == -1){
      perror("accept error");
      return 1;
    }
    read(peer_socket, in_buf, 1023);

    if(strncmp(in_buf, "nasdaq", 6) == 0){
      sprintf(in_buf, "%u", 22563);
    }else{
      unsigned long time_now = time(NULL);
      sprintf(in_buf, "%lu", time_now+HK_OFFSET);
    }
    send(peer_socket, in_buf, strlen(in_buf), 0);
    close(peer_socket);
  }


}
