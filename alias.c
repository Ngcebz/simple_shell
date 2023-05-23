#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "main.h"

#define MAX_INPUT_LENGTH 1024
#define MAX_ALIAS_COUNT 100

typedef struct {
    char name[MAX_INPUT_LENGTH];
    char value[MAX_INPUT_LENGTH];
} Alias;

Alias aliases[MAX_ALIAS_COUNT];
int alias_count = 0;

/**
 * print_prompt - prints shell prompt
 */
void print_prompt(void)
{
    write(STDOUT_FILENO, "$ ", 2);
}

/**
 * read_input - reads input from user
 * @input: buffer to store user input
 * Return: size of the input read
 */
ssize_t read_input(char *input)
{
    ssize_t read_size = read(STDIN_FILENO, input, MAX_INPUT_LENGTH);

    if (read_size == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    else if (read_size == 0)
    {
        write(STDOUT_FILENO, "\n", 1);
        exit(EXIT_SUCCESS);
    }
    /* Remove newline character from end of command */
    input[read_size - 1] = '\0';
    return (read_size);
}

/**
 * execute_command - executes command
 * @command: command to execute
 */
void execute_command(char *command)
{
    pid_t pid = fork();
    int status;

    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        /* Child process */
        /* Execute command */
        if (access(command, F_OK) != -1)
        {
            execve(command, NULL, environ);
            perror("execve");
            _exit(EXIT_FAILURE);
        }
        else
        {
            write(STDERR_FILENO, "Command not found\n", 18);
            _exit(EXIT_FAILURE);
        }
    }
    else
    {
        /* Parent process */
        /* Wait for child process to complete */
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

/**
 * print_aliases - prints all aliases
 */
void print_aliases(void)
{
    int i;
    for (i = 0; i < alias_count; i++)
    {
        printf("%s='%s'\n", aliases[i].name, aliases[i].value);
    }
}

/**
 * print_alias - prints aliases by name
 * @alias_names: array of alias names
 * @count: number of alias names
 */
void print_alias(char **alias_names, int count)
{
    int i, j;
    for (i = 0; i < count; i++)
    {
        for (j = 0; j < alias_count; j++)
        {
            if (strcmp(alias_names[i], aliases[j].name) == 0)
            {
                printf("%s='%s'\n", aliases[j].name, aliases[j].value);
                break;
            }
        }
    }
}

/**
 * add_alias - adds or updates an alias
 * @name: alias name
 * @value: alias value
 */
void add_alias(char *name, char *value)
{
    int i;
    for (i = 0; i < alias_count; i++)
    {
        if (strcmp(name, aliases[i].name) == 0)
        {
            strcpy(aliases[i].value, value);
            return;
        }
    }

    if (alias_count < MAX_ALIAS_COUNT)
    {
        strcpy(aliases[alias_count].name, name);
        strcpy(aliases[alias_count].value, value);
        alias_count++;
    }
    else
    {
        fprintf(stderr, "Too many aliases. Cannot add alias.\n");
    }
}

/**
 * process_alias_command - processes the alias command
 * @args: command arguments
 * @arg_count: number of arguments
 */
void process_alias_command(char **args, int arg_count)
{
    if (arg_count == 0)
    {
        // Print all aliases
        print_aliases();
    }
    else if (arg_count == 1)
    {
        // Print aliases by name
        print_alias(&args[1], arg_count - 1);
    }
    else
    {
        // Add or update aliases
        int i;
        for (i = 1; i < arg_count; i++)
        {
            char *arg = args[i];
            char *name = strtok(arg, "=");
            char *value = strtok(NULL, "=");

            if (name != NULL && value != NULL)
            {
                add_alias(name, value);
            }
            else
            {
                fprintf(stderr, "Invalid alias format: %s\n", arg);
            }
        }
    }
}

/**
 * main - shell program that reads and executes
 * simple one-word commands from the user.
 * @argc: argument count
 * @argv: argument vector
 * Return: 0 on success, non-zero on failure
 */
int main(int argc, char **argv)
{
    char input[MAX_INPUT_LENGTH];
    char *args[MAX_INPUT_LENGTH / 2 + 1];
    char **environ;
    int arg_count;

    while (1)
    {
        print_prompt();
        ssize_t read_size = read_input(input);

        // Tokenize the input into arguments
        arg_count = 0;
        char *token = strtok(input, " ");
        while (token != NULL)
        {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        // Check if the command is 'alias'
        if (arg_count > 0 && strcmp(args[0], "alias") == 0)
        {
            process_alias_command(args, arg_count);
        }
        else
        {
            execute_command(args[0]);
        }
    }
    return (0);
}
