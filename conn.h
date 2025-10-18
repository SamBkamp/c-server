#ifndef OPEN_CONNECTION
#define OPEN_CONNECTION

int open_connection(uint16_t PORT, connection_info *ret);
int request_stock_data(char* response_str, size_t str_size, char* endpoint, char* symbol);

#endif
