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
#include <pthread.h>

#include "prot.h"
#include "conn.h"
#include "parse.h"

#define CACHE_TTL 600 //cache hold time in seconds
#define PORT 8080
#define HK_OFFSET 28800 //8 hours in seconds

#define CACHE_TEM 0
#define CACHE_XAU 1
#define CACHE_MKT 2
#define CACHE_TTWO 3
#define CACHE_QCOM 4
quote_cache cache[5];

char* long_to_ip(char* out, unsigned long IP){
  memset(out, 0, 16); //16 bytes max for an IP string (with nullptr)
  size_t out_idx = 0;
  for(size_t i = 0; i < 3; i++){
    out_idx += sprintf(&out[out_idx], "%d.", ((unsigned char*)&IP)[i]);
  }
  out_idx += sprintf(&out[out_idx], "%d", ((unsigned char*)&IP)[3]); //last digit has no trailing .
  return out;
}

//the value of array (the pointer) doesn't change
//returns first element of an array
//TODO make this like another custom strtok to give all elements of an array
char* take_first_elment(char* array){
  char* start = array;
  //hack to deal with javascript arrays contiaining objects
  if(*start == '['){ //if first character is array opener
    uint8_t in_object = 0;
    uint8_t in_string = 0;
    while((*start != ','&&*start != ']')
          || in_object != 0 || in_string > 0){
      switch(*start){
      case '"':
        in_string = (in_string+1)%2;
        break;
      case '{':
        in_object++;
        break;
      case '}':
        in_object--;
        break;
      }
      start++;
    }
    *start = 0;
    start = array+1;
  }

  return start;
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
  json_parse(take_first_elment(res.body), pairs); //parses the response json

  cache[symbol].data = pairs;
  cache[symbol].timestamp = time_now;
  free(res.body);
  return pairs;
}

void populate_cache(int cache_idx, char *argument, char* endpoint){
  cache[cache_idx].timestamp = 0;
  strncpy(cache[cache_idx].arguments,
          argument,
          sizeof(cache[cache_idx].arguments));
  strncpy(cache[cache_idx].endpoint,
          endpoint,
          sizeof(cache[cache_idx].endpoint));
}

void init_caches(){
  populate_cache(CACHE_TEM, "symbol=TEM&interval=1day&dp=2&rolling_period=24", "quote");
  populate_cache(CACHE_TTWO, "symbol=TTWO&interval=1day&dp=2&rolling_period=24", "quote");
  populate_cache(CACHE_QCOM, "symbol=QCOM&interval=1day&dp=2&rolling_period=24", "quote");
  populate_cache(CACHE_XAU, "symbol=XAU/USD", "exchange_rate");
  populate_cache(CACHE_MKT, "code=ARCX", "market_state");

}

int connection_worker(void *ptr){
  char in_buf[2048]; //buffer for inbound connections
  peer_connection *pc = (peer_connection*) ptr;
  char inbd_ip[16];
  ssize_t r;

  printf("new connection from %s\n", long_to_ip(inbd_ip, pc->peer_addr.sin_addr.s_addr));
  r = read(pc->sockfd, in_buf, 1023);

  if(r < 0){
    perror("read() error");
    close(pc->sockfd);
    free(pc);
    return 1;
  }

  //TODO: consider turning this into a hashmap? lengthy if/else if/else is ugly and inefficient
  if(strncmp(in_buf, "/q", 2) == 0){
    kv_pair *pairs = quote_request(CACHE_TEM);
    sprintf(in_buf, "TEM:\n%s   %s%%", pairs[11].value, pairs[15].value);
  }else if(strncmp(in_buf, "/gold", 5) == 0){
    kv_pair *pairs = quote_request(CACHE_XAU);
    kv_pair *market_pairs = quote_request(CACHE_MKT);
    //this FUCKING SUCKS
    int compare = strcmp(market_pairs[3].value, "true");
    sprintf(in_buf, "GOLD: $%s\nmkt: %s", pairs[1].value, compare==0?"open":"closed");
    fflush(stdout);
  }else if(strncmp(in_buf, "/all", 4)==0){
    kv_pair *tem_pair = quote_request(CACHE_TEM);
    kv_pair *ttwo_pair = quote_request(CACHE_TTWO);
    sprintf(in_buf, "TEM:\n%s   %s%%\tTTWO:\n%s   %s%%", tem_pair[11].value, tem_pair[15].value, ttwo_pair[11].value, ttwo_pair[15].value);
  }else{
    unsigned long time_now = time(NULL);
    sprintf(in_buf, "%lu", time_now+HK_OFFSET);
  }
  send(pc->sockfd, in_buf, strlen(in_buf), 0);
  close(pc->sockfd);
  free(pc);
  return 0;
}

int main(){
  connection_info ci;
  pthread_t threads;

  init_caches();

  if(open_connection(PORT, &ci) != 0) //open local listening socket
    return 1;
  printf("Started server on port %d\n", PORT);


  //main event loop
  while(1){
    peer_connection *pc = malloc(sizeof(peer_connection));
    pc->size_of_peer = sizeof(pc->peer_addr);
    pc->sockfd = accept(ci.sockfd, (struct sockaddr *)&pc->peer_addr, &pc->size_of_peer);

    if(pc->sockfd < 0){
      perror("accept error");
      free(pc);
    }
    int thread = pthread_create(&threads, NULL, (void *)connection_worker, pc);
    if(thread != 0){
      perror("thread create error");
      free(pc);
    }
  }
  printf("you should not be here\n");
}
