//
// Created by daniel on 4/27/17.
//

#include <glob.h>

#ifndef UNTITLED_PARSER_HEADERS_H
#define UNTITLED_PARSER_HEADERS_H

char* parse_path(char* uri);
void view_string(char* string);
void read_response_write_headers(int clientfd,int proxy_clientfd);

#endif //UNTITLED_PARSER_HEADERS_H
