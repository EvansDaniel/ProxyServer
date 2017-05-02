#include <time.h>
#include "csapp.h"

//
// Created by daniel on 5/1/17.
//

#ifndef UNTITLED_SERVER_H
#define UNTITLED_SERVER_H

#include "csapp.h"

void* process_request(void* fd);

int read_request_headers(rio_t *rp, char* headers);
char* parse_path(char* uri);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);
void cleanup(int peer_serverfd,char* headers,char* path,char* host_p, void* connfd);

#endif //UNTITLED_SERVER_H


//
// Created by daniel on 4/27/17.
//

#include <glob.h>

#ifndef UNTITLED_PARSER_HEADERS_H
#define UNTITLED_PARSER_HEADERS_H

char* parse_path(char* uri);
void view_string(char* string,int len);
ssize_t read_response_write_client(int clientfd, int proxy_clientfd);
void add_header(char* header, char* headers);
char* try_prepending_www(char *p);
char* parse_host(char* host_header);
char* new_parse_host(char* uri);
#endif //UNTITLED_PARSER_HEADERS_H


//
// Created by daniel on 5/1/17.
//

#ifndef UNTITLED_HELPERS_H
#define UNTITLED_HELPERS_H

/* $end tinymain */
void remove_all_chars(char* str, char c);
void write_bs(int fd);
int min(int a, int b);
int max(int a, int b);

#endif //UNTITLED_HELPERS_H


//
// Created by daniel on 5/1/17.
//

#ifndef UNTITLED_LOG_H
#define UNTITLED_LOG_H


// Might need to pass the file descriptor to write to?
// Would that be thread safe though?
int log_response(struct sockaddr_in *sockaddr, char *uri, ssize_t size);

#endif //UNTITLED_LOG_H




