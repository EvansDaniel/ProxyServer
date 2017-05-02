//
// Created by daniel on 4/27/17.
//

#include <glob.h>

#ifndef UNTITLED_PARSER_HEADERS_H
#define UNTITLED_PARSER_HEADERS_H

char* parse_path(char* uri);
void view_string(char* string);
int read_response_write_client(int clientfd, int proxy_clientfd);
void add_header(char* header, char* headers);
char* try_prepending_www(char *p);
char* parse_host(char* host_header);

#endif //UNTITLED_PARSER_HEADERS_H
