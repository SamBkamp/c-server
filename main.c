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

#define CACHE_TEM 0
#define CACHE_XAU 1
#define CACHE_MKT 2

quote_cache cache[3];

//formats a number from json to 2 sig. fig. (eg 604.83990 -> 604.83) (no rounding, yet)
void format_2sf(char* in){
  size_t decimal_index = 0;
  while(in[decimal_index] != '.' && in[decimal_index] != 0){
    decimal_index++;
  }
  if(strlen(in) >= decimal_index+3)
    in[decimal_index+3] = 0;
}

char* long_to_ip(char* out, unsigned long IP){
  memset(out, 0, 16); //16 bytes max for an IP string (with nullptr)
  size_t out_idx = 0;
  for(size_t i = 0; i < 3; i++){
    out_idx += sprintf(&out[out_idx], "%d.", ((unsigned char*)&IP)[i]);
  }
  out_idx += sprintf(&out[out_idx], "%d", ((unsigned char*)&IP)[3]); //last digit has no trailing .
  return out;
}

//handler function for quote requests, manages cache and sends new requests as needed
kv_pair *quote_request(size_t symbol){
  unsigned long time_now = time(NULL);
  if(time_now <=  cache[symbol].timestamp + CACHE_TTL){
    printf("using cached data for %s\n", cache[symbol].endpoint);
    return cache[symbol].data;
  }
  printf("cache for %s expired, making new request\n", cache[symbol].endpoint);

  if(cache[symbol].timestamp != 0)
    free(cache[symbol].data);

  char response_buff[2048];
  http_response res = {0};
  kv_pair *pairs = malloc(20*sizeof(kv_pair));
  request_stock_data(response_buff, 2048,
		     cache[symbol].endpoint, cache[symbol].arguments); //sends https request to API, raw data placed in response buff
  parse_http_response(&res, response_buff); //parses the http response
  printf("%s\n", res.body);
  json_parse(res.body, pairs); //parses the response json

  cache[symbol].data = pairs;
  cache[symbol].timestamp = time_now;
  free(res.body);
  return pairs;
}

void init_cache(){
  cache[CACHE_TEM].timestamp = 0;
  strncpy(cache[CACHE_TEM].arguments,
	  "symbol=TEM&interval=30min&dp=2",
	  sizeof(cache[CACHE_TEM].arguments));
  strncpy(cache[CACHE_TEM].endpoint, "quote", 20);

  cache[CACHE_XAU].timestamp = 0;
  strncpy(cache[CACHE_XAU].arguments,
	  "symbol=XAU/USD",
	  sizeof(cache[CACHE_XAU].arguments));
  strncpy(cache[CACHE_XAU].endpoint, "exchange_rate", 20);

  cache[CACHE_MKT].timestamp = 0;
  strncpy(cache[CACHE_MKT].arguments,
	  "code=ARCX",
	  sizeof(cache[CACHE_MKT].arguments));
  strncpy(cache[CACHE_MKT].endpoint, "market_state", 20);

  if(open_connection(PORT, &ci) != 0) //open local listening socket
    return 1;
  printf("Started server on port %d\n", PORT);
  socklen_t size_of_peer = sizeof(ci.peer_addr);

  //main even loop
  while(1){
    int peer_socket = accept(ci.sockfd, (struct sockaddr *)&ci.peer_addr, &size_of_peer);
    char inbd_ip[16];
    printf("new connection from %s\n", long_to_ip(inbd_ip, ci.peer_addr.sin_addr.s_addr));

    if(peer_socket == -1){
      perror("accept error");
      return 1;
    }
    read(peer_socket, in_buf, 1023);

    //TODO: consider turning this into a hashmap? lengthy if/else if/else is ugly and inefficient
    if(strncmp(in_buf, "/q", 2) == 0){
      kv_pair *pairs = quote_request(CACHE_TEM);
      format_2sf(pairs[13].value);
      format_2sf(pairs[14].value);
      sprintf(in_buf, "TEM:\n%s %s%%", pairs[13].value, pairs[15].value);
    }else if(strncmp(in_buf, "/gold", 5) == 0){
      kv_pair *pairs = quote_request(CACHE_XAU);
      kv_pair *market_pairs = quote_request(CACHE_MKT);
      format_2sf(pairs[1].value);
      char convert[8]; //convert true/false to open/close (ik its hacky)
      int compare = strcmp(market_pairs[3].value, "true");
      strcpy(convert, compare==0?"open":"closed");
      sprintf(in_buf, "GOLD: $%s\nmkt: %s", pairs[1].value, convert);
      fflush(stdout);
    }else{
      unsigned long time_now = time(NULL);
      sprintf(in_buf, "%lu", time_now+HK_OFFSET);
    }
    send(peer_socket, in_buf, strlen(in_buf), 0);
    close(peer_socket);
  }


}
