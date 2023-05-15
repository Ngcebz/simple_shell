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
void execute_command(char **args);
void wait_for_child(pid_t pid, int *status);
void handle_command_execution(char **args);
void parse_command(char *input, char **args);

#endif /* MAIN_H */
