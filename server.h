//
// Created by daniel on 5/1/17.
//

#ifndef UNTITLED_SERVER_H
#define UNTITLED_SERVER_H

#include "csapp.h"

void process_request(int fd);

char* read_request_headers(rio_t *rp, char* headers);
char* parse_path(char* uri);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);

#endif //UNTITLED_SERVER_H
