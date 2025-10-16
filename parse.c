#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prot.h"
#include "parse.h"

int in_string = 0; //keeps track if we are in a string 0 = false
int sub_obj = 0; //keeps track if were inbetween {}
size_t start = 0;
size_t s_len = 0;
char* working_s;
//functions like strtok but doesnt split when the delim is in a string or between {}
char* custom_strtok(char* s, char delim){
  int ret_idx = 0;
  if(s != NULL){ //init our strtok
    s_len = strlen(s);
    start = 0;
    working_s = s;
  }
  if(start == s_len) //reached the end of our string already
    return NULL;
  for(size_t i = start; i < s_len; i++){
    switch(working_s[i]){
    case '"':
      in_string = (in_string+1)%2;
      break;
    case '{':
      sub_obj++;
      break;
    case '}':
      sub_obj--;
      break;
    }
    if((working_s[i]==delim) //check for delim
       && in_string == 0 && sub_obj == 0){ //and not in string or obj
      working_s[i] = 0;
      ret_idx = start;
      start = i+1;
      return &working_s[ret_idx];
    }
  }
  //only reaches here if at the end of string
  ret_idx = start;
  start = s_len; //next call will NULL
  return &working_s[ret_idx];
}

void json_parse(char* in_string, kv_pair* pairs){
  in_string[strlen(in_string)-1] = 0;//strip trailing }
  in_string++; //strip leading {
  char* token = custom_strtok(in_string, ',');
  while(token != NULL){ 
    size_t delim = 0;
    while(token[delim] != ':' && token[delim] != 0) //find : character to split key and value
      delim++;
    token[delim] = 0; //null terminator to split the strings
    
    strncpy(pairs->key, token, VAL_MAX);
    strncpy(pairs->value, &token[delim+1], VAL_MAX);
    pairs++; //OOB RISK, im not managing memory for u
    token = custom_strtok(NULL, ',');
  }
}


int parse_http_response(http_response *res, char* data){
  const char* delim = "\n";
  char* token = strtok(data, delim);

  while(token != NULL){
    //check for http code
    //some APIS return 0x1c (FS) at the start of their http response
    if(strncmp(token, "HTTP", 4)==0 || strncmp(token+1, "HTTP", 4)==0){
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
