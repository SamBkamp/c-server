#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <error.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "api_secret.h"

#define HOST "api.twelvedata.com"

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

int main(void) {
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

  char request[512];
  const char* end_point = "quote";
  const char* ticker = "TEM";  
  snprintf(request, 512, "GET /%s?symbol=%s&apikey=%s HTTP/1.1\r\nHost: api.twelvedata.com\r\nConnection: close\r\n\r\n", end_point, ticker, API_SECRET);

  SSL_write(ssl, request, strlen(request));
  
  int bytes_received;
  char buffer[1024];
  while ((bytes_received = SSL_read(ssl, buffer, 1023)) > 0) {
    buffer[bytes_received] = '\0';
    printf("%s", buffer);
  }
  SSL_shutdown(ssl);
  SSL_free(ssl);
  close(out_sockfd);
  SSL_CTX_free(ctx);
}

