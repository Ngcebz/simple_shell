#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "main.h"

#define MAX_INPUT_LENGTH 1024

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
 * set_env_variable - set or modify an environment variable
 * @variable: name of the variable
 * @value: value to set for the variable
 * Return: 0 on success, -1 on failure
 */
int set_env_variable(const char *variable, const char *value)
{
	if (setenv(variable, value, 1) != 0)
	{
		perror("setenv");
		return -1;
	}
	return 0;
}

/**
 * unset_env_variable - unset an environment variable
 * @variable: name of the variable to unset
 * Return: 0 on success, -1 on failure
 */
int unset_env_variable(const char *variable)
{
	if (unsetenv(variable) != 0)
	{
		perror("unsetenv");
		return -1;
	}
	return 0;
}

/**
 * handle_builtin_commands - handle builtin commands
 * @args: command arguments
 * Return: 1 if command is builtin, 0 otherwise
 */
int handle_builtin_commands(char **args)
{
	if (strcmp(args[0], "setenv") == 0)
	{
		if (args[1] != NULL && args[2] != NULL)
		{
			if (set_env_variable(args[1], args[2]) != 0)
				return 0;
		}
		else
		{
			write(STDERR_FILENO, "Usage: setenv VARIABLE VALUE\n", 29);
		}
		return 1;
	}
	else if (strcmp(args[0], "unsetenv") == 0)
	{
		if (args[1] != NULL)
		{
			if (unset_env_variable(args[1]) != 0)
				return 0;
		}
		else
		{
			write(STDERR_FILENO, "Usage: unsetenv VARIABLE\n", 25);
		}
		return 1;
	}

	return 0;
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
	char *args[MAX_INPUT_LENGTH];
	char **environ;
	int is_builtin_command;

	while (1)
	{
		print_prompt();
		ssize_t read_size = read_input(input);

		if (read_size == 0)
			continue;

		is_builtin_command = 0;
		parse_command(input, args);

		is_builtin_command = handle_builtin_commands(args);

		if (!is_builtin_command)
			execute_command(args[0]);
	}

	return (0);
}
