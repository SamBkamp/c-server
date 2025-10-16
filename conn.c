#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "prot.h"
#include "conn.h"
#include "api_secret.h"

#define HOST "api.twelvedata.com"

//internal function, returns pointer to SSL context
SSL_CTX *init_ssl_context(){
   // Initialize OpenSSL
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();

  SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
  if (!ctx) {
    ERR_print_errors_fp(stderr);
    return (SSL_CTX*)-1;
  }
  return ctx;
}

//internal function, returns fd of our outbound socket
int init_socket(){
  //set up the socket
  //this function gets remote addr info, requests the socket, and connects
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int gai = getaddrinfo(HOST, "443", &hints, &res);
  if(gai != 0){
    printf("%s\n", gai_strerror(gai));
    return -1;
  }

  int out_socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if(out_socketfd == -1){
    perror("socket error");
    freeaddrinfo(res);
    return -1;
  }

  if(connect(out_socketfd, res->ai_addr, res->ai_addrlen) < 0){
    perror("connect");
    freeaddrinfo(res);
    return -1;
  }
  freeaddrinfo(res);
  return out_socketfd;
}

int request_stock_data(char* response_str, size_t str_size) {
  int bytes_recieved;
  int total_bytes_read = 0;
  char buffer[1024]; //response data buffer
  char request[512]; //request data buffer
  SSL_CTX *ctx;
  SSL *ssl;

  ctx = init_ssl_context();
  if(ctx == (SSL_CTX *)-1)
    return 1;

  int out_sockfd = init_socket();
  if(out_sockfd < 0)
    return 1;

  //use the SSL context to init SSL
  ssl = SSL_new(ctx);
  SSL_set_fd(ssl, out_sockfd);
  SSL_set_tlsext_host_name(ssl, HOST); //have to set hostname for servers with multiple hosts, so you get the correct SSL Certs

  if (SSL_connect(ssl) <= 0) {
    printf("SSL connect: ");
    ERR_print_errors_fp(stderr);
    SSL_free(ssl);
    close(out_sockfd);
    SSL_CTX_free(ctx);
    return 1;
  }

  const char* end_point = "quote";
  const char* ticker = "TEM";
  snprintf(request, 512, "GET /%s?symbol=%s&apikey=%s HTTP/1.1\r\nHost: api.twelvedata.com\r\nConnection: close\r\n\r\n", end_point, ticker, API_SECRET);

  SSL_write(ssl, request, strlen(request));

  while ((bytes_recieved = SSL_read(ssl, buffer, 1023)) > 0) {
    buffer[bytes_recieved] = '\0';
    if(total_bytes_read+bytes_recieved < (int)str_size){
      strcat(response_str, buffer);
      total_bytes_read+=bytes_recieved;
    }
    else
      break;
  }
  SSL_shutdown(ssl);
  SSL_free(ssl);
  close(out_sockfd);
  SSL_CTX_free(ctx);
  return 0;
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
