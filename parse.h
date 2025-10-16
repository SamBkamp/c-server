#ifndef PARSE
#define PARSE

#define VAL_MAX 512
typedef struct{
  char key[VAL_MAX];
  char value[VAL_MAX];
}kv_pair;


int parse_http_response(http_response *res, char* data);
void json_parse(char* in_string, kv_pair* pairs);

#endif
