#ifndef UNTITLED_PARSER_HEADERS_H
#define UNTITLED_PARSER_HEADERS_H

#include <glob.h>

char* parse_path(char* uri);
char *parse_path_helper(char *uri);
char* parse_host_from_headers(char* headers);
// helper function for debugging strings
void view_string(char* string,int len);
ssize_t read_response_write_client(int clientfd, int proxy_clientfd);
void add_header(char* header, char* headers);
char* parse_host(char* host_header);


#endif //UNTITLED_PARSER_HEADERS_H
