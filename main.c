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

#define CACHE_TTL 600 //cache hold time in seconds
#define PORT 8080
#define HK_OFFSET 28800 //8 hours in seconds

quote_cache cache;

void quote_request(kv_pair *pairs){
  unsigned long time_now = time(NULL);
  if(time_now <=  cache.timestamp + CACHE_TTL){
    printf("using cached data\n");
    pairs = cache.data;
    return;
  }
  printf("cache expired, making new request\n");
  char response_buff[2048]; //response from outbound connection
  http_response res = {0};
  request_stock_data(response_buff, 2048);
  parse_http_response(&res, response_buff);
  json_parse(res.body, pairs);
  cache.data = pairs;
  cache.timestamp = time_now;
  free(res.body);
}


int main(){
  connection_info ci;
  char in_buf[1024]; //buffer for inbound connections
  kv_pair pairs[20];
  //init cache
  cache.timestamp = 0;

  if(open_connection(PORT, &ci) != 0){
    return 1;
  }

  printf("Started server on port %d\n", PORT);

  socklen_t size_of_peer = sizeof(ci.peer_addr);
  while(1){
    int peer_socket = accept(ci.sockfd, (struct sockaddr *)&ci.peer_addr, &size_of_peer);
    printf("new connection from %x\n", ntohl(ci.peer_addr.sin_addr.s_addr));

    if(peer_socket == -1){
      perror("accept error");
      return 1;
    }
    read(peer_socket, in_buf, 1023);

    if(strncmp(in_buf, "qqq", 3) == 0){
      quote_request(pairs);
      sprintf(in_buf, "%s", pairs[0].value);
      fflush(stdout);
    }else{
      unsigned long time_now = time(NULL);
      sprintf(in_buf, "%lu", time_now+HK_OFFSET);
    }
    send(peer_socket, in_buf, strlen(in_buf), 0);
    close(peer_socket);
  }


}
