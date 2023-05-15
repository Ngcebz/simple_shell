#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void print_prompt(void);
ssize_t read_input(char *input);
void execute_command(char *command);

#endif /* MAIN_H */
