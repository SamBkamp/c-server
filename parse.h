#ifndef PARSE
#define PARSE

int parse_http_response(http_response *res, char* data);
void json_parse(char* in_string, kv_pair* pairs);

#endif //PARSE
