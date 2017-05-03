#include <time.h>
#include "csapp.h"

#ifndef UNTITLED_SERVER_H
#define UNTITLED_SERVER_H

void* process_request(void* fd);
int read_request_headers(rio_t *rp, char* headers);
char* parse_path(char* uri);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void cleanup(int peer_serverfd,char* headers,char* path,char* host_p, void* connfd);

#endif

#include "parser_headers.h"
#include "helpers.h"
#include "log.h"

