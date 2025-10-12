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

#define SECONDS_PER_HOUR 3600
#define HK_OFFSET 28800 //8 hours

void format_http_response(http_response *res, char* str, size_t limit){
  char* res_str = "\
HTTP/2 404\r\n\
Content-Type: text/html\r\n\
Content-Length: 21\r\n\
Connection: Closed\r\n\r\n\
<h1>Hello world</h1>\r\n";

  strncpy(str, res_str, limit);
}

int main(int argc, char* argv[]){
  connection_info ci;
  char in_buf[1024];

  if(open_connection(8080, &ci) != 0){
    return 1;
  }

  socklen_t size_of_peer = sizeof(ci.peer_addr);
  while(1){
    int peer_socket = accept(ci.sockfd, (struct sockaddr *)&ci.my_addr, &size_of_peer);

    if(peer_socket == -1){
      perror("accept error");
      return 1;
    }

    http_response res = {0};
    res.code = 200;
    res.content_type = "text/html";

    memset(in_buf, 0, 1023);
    ssize_t in_val = read(peer_socket, in_buf, 1023);
    printf("%s\n", in_buf);
    memset(in_buf, 0, 1023);
    unsigned long time_now = time(NULL);
    sprintf(in_buf, "%lu", time_now+HK_OFFSET);
    printf("%s\n", in_buf);
    send(peer_socket, in_buf, strlen(in_buf), 0);
  }


}
