#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prot.h"
#include "parse.h"

typedef struct{
  char key[20];
  char value[20];
}kv_pair;

int in_string = 0; //keeps track if we are in a string 0 = false
int sub_obj = 0; //keeps track if were inbetween {}
size_t start = 0;
size_t s_len = 0;
char* working_s;
//functions like strtok but doesnt split when the delim is in a string or between {}
char* custom_strtok(char* s, char delim){
  int ret_idx = 0;
  if(s != NULL){
    s_len = strlen(s);
    start = 0;
    working_s = s;
  }

  if(start == s_len)
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
  start = s_len;
  return &working_s[ret_idx];
}

void json_parse(){
  const char *str = "{\"symbol\":\"TEM\",\"name\":\"Tempus AI, Inc.\",\"exchange\":\"NASDAQ\",\"mic_code\":\"XNGS\",\"currency\":\"USD\",\"datetime\":\"2025-10-14\",\"timestamp\":1760448600,\"last_quote_at\":1760448600,\"open\":\"92.11000\",\"high\":\"93.90600\",\"low\":\"87.30000\",\"close\":\"89.87000\",\"volume\":\"6952000\",\"previous_close\":\"94.47000\",\"change\":\"-4.60000\",\"percent_change\":\"-4.86927\",\"average_volume\":\"8553950\",\"is_market_open\":false,\"fifty_two_week\":{\"low\":\"31.36000\",\"high\":\"104.32000\",\"low_change\":\"58.51000\",\"high_change\":\"-14.45000\",\"low_change_percent\":\"186.57526\",\"high_change_percent\":\"-13.85161\",\"range\":\"31.360001 - 104.320000\"}}";
  char *str_set = malloc((strlen(str)+1)*sizeof(char));
  strcpy(str_set, str);


  size_t str_len_set = strlen(str_set);
  str_set[--str_len_set] = 0;//strip trailing } (TODO: check if last char is actually closing brace)
  str_set++; //strip leading { (TODO: check todo above)
  //works like a custom strtok that ignores delim if certain conditions are met
  // in this case if the delim is in a string
  char* token = custom_strtok(str_set, ',');
  while(token != NULL){
    printf("token: %s\n", token);
    token = custom_strtok(NULL, ',');
  }
}


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
