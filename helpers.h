//
// Created by daniel on 5/1/17.
//

#ifndef UNTITLED_HELPERS_H
#define UNTITLED_HELPERS_H

/* $end tinymain */
void remove_all_chars(char* str, char c);

int CRITICAL_CODE_SECTION_BEGIN(sem_t *sem);

int CRITICAL_CODE_SECTION_END(sem_t *sem);

#endif //UNTITLED_HELPERS_H
