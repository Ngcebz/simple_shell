#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "main.h"

#define MAX_INPUT_LENGTH 1024

extern char **environ;

void print_prompt(void)
{
	write(STDOUT_FILENO, "$ ", 2);
}

ssize_t read_command(char *input)
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

int count_args(char *input)
{
	int count = 0;
	int in_word = 0;

	for (int i = 0; input[i] != '\0'; i++)
	{
		if (input[i] != ' ' && input[i] != '\t' && input[i] != '\n')
		{
			if (in_word == 0)
			{
				in_word = 1;
				count++;
			}
		}
		else
		{
			in_word = 0;
		}
	}

	return count;
}

void parse_command(char *input, char **args)
{
	int arg_index = 0;
	int arg_len = 0;
	int in_word = 0;

	for (int i = 0; input[i] != '\0'; i++)
	{
		if (input[i] != ' ' && input[i] != '\t' && input[i] != '\n')
		{
			if (in_word == 0)
			{
				in_word = 1;
				args[arg_index] = &input[i];
				arg_index++;
			}
		}
		else
		{
			in_word = 0;
			input[i] = '\0';
		}

		arg_len++;
	}

	/* Null-terminate the last argument */
	args[arg_index] = NULL;
}

void execute_command(char **args)
{
	if (access(args[0], F_OK) != -1)
	{
		execve(args[0], args, environ);
		perror("execve");
		_exit(EXIT_FAILURE);
	}
	else
	{
		write(STDERR_FILENO, "Command not found\n", 18);
		_exit(EXIT_FAILURE);
	}
}

void wait_for_child(pid_t pid, int *status)
{
	do {
		waitpid(pid, status, WUNTRACED);
	} while (!WIFEXITED(*status) && !WIFSIGNALED(*status));
}

void handle_command_execution(char **args)
{
	pid_t pid;
	int status;

	/* Create child process */
	pid = fork();

	if (pid == -1)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		/* Child process */

		/* Execute command */
		execute_command(args);
	}
	else
	{
		/* Parent process */

		/* Wait for child process to complete */
		wait_for_child(pid, &status);
	}
}

int main(int argc, char **argv)
{
	char input[MAX_INPUT_LENGTH];
	char *args[MAX_INPUT_LENGTH];

	while (1)
	{
		/* Print prompt */
		print_prompt();

		/* Read command from user */
		ssize_t read_size = read_command(input);

		/* Parse command into arguments */
		parse_command(input, args);

		/* Handle command execution */
		handle_command_execution(args);
	}

	return (0);
}
