//
// Created by daniel on 5/1/17.
//

#ifndef UNTITLED_LOG_H
#define UNTITLED_LOG_H


// Might need to pass the file descriptor to write to?
// Would that be thread safe though?
void log_response(struct sockaddr_in *sockaddr, char *uri, int size);

#endif //UNTITLED_LOG_H
