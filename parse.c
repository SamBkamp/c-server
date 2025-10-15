#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prot.h"
#include "parse.h"

int parse_http_response(http_response *res, char* data){
  const char* delim = "\n";
  char* token = strtok(data, delim);
  
  while(token != NULL){
    //check for http code
    if(strncmp(token, "HTTP", 4)==0){
      char code[5];
      for(int i = 0; i < 4; i++)
	code[i] = token[9+i];
      //HTTP code comes at the 10th char (index = 9)
      //response codes are always 3 digits
      code[4] = 0;
      res->response_code = atoi(code);
    }
    
    //check for content length
    if(strncmp(token, "Content-Length", 14)==0){
      char c_len[10]; //max 10 digits I guess
      for(int i = 0; i < 9; i++)
	c_len[i] = token[16+i];
      c_len[9] = 0;
      res->content_length = atoi(c_len);
    }

    //read body data
    if(strncmp(token, "\r", 2)==0){
      token = strtok(NULL, delim); //get next line
      if(token != NULL){
	size_t body_len = strlen(token);
	res->body = malloc(sizeof(char)*body_len);
	strcpy(res->body, token); //this SHOULD never oob
	if(res->body[body_len-1] == '\n') //remove newline
	  res->body[body_len-1] = 0;
      }
    }
    token = strtok(NULL, delim);
  }
  return 0;
}

