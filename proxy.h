//
// Created by daniel on 4/27/17.
//

#ifndef UNTITLED_PROXY_H
#define UNTITLED_PROXY_H

void doit(int fd);


int parse_uri(char *uri, char *filename, char *cgiargs);

void serve_static(int fd, char *filename, int filesize);

void get_filetype(char *filename, char *filetype);

void serve_dynamic(int fd, char *filename, char *cgiargs);

void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);

char *parse_path_from_uri(char uri[]);

char *parse_path_from_uri(char *uri);

#endif //UNTITLED_PROXY_H
