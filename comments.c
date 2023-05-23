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

	/* Check for comment and remove it */
	char *comment = strchr(input, '#');
	if (comment != NULL)
		*comment = '\0';

	return strlen(input);
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
 * main - shell program that reads and executes
 * simple one-word commands from the user.
 * @argc: argument count
 * @argv: argument vector
 * Return: 0 on success, non-zero on failure
 */
int main(int argc, char **argv)
{
	char input[MAX_INPUT_LENGTH];
	environ = argv;  /* Initialize environ with the passed argument vector */

	while (1)
	{
		print_prompt();
		ssize_t read_size = read_input(input);

		/* Skip empty commands */
		if (strlen(input) == 0)
			continue;

		execute_command(input);
	}

	return 0;
}
