#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "main.h"

extern char **environ;

#define MAX_INPUT_LENGTH 1024
#define MAX_NUM_ARGS 128

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

	return read_size;
}

void parse_command(char *input, char **args)
{
	char *arg = strtok(input, " ");
	int i = 0;

	while (arg != NULL && i < MAX_NUM_ARGS - 1)
	{
		args[i++] = arg;
		arg = strtok(NULL, " ");
	}
	args[i] = NULL;
}

int check_command_exists(char *command)
{
	if (access(command, F_OK) == -1)
	{
		write(STDERR_FILENO, "Command not found\n", 18);
		return 0;
	}
	return 1;
}

void execute_command(char **args)
{
	execve(args[0], args, environ);
	perror("execve");
	_exit(EXIT_FAILURE);
}

void wait_for_child(pid_t pid, int *status)
{
	do
	{
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
	char *args[MAX_NUM_ARGS];

	while (1)
	{
		/* Print prompt */
		print_prompt();

		/* Read command from user */
		ssize_t read_size = read_command(input);

		/* Check if command is "exit" */
		if (strcmp(input, "exit") == 0)
		{
			int exit_status = EXIT_SUCCESS;

			/* Check for an optional status argument */
			if (args[1] != NULL)
			{
				exit_status = atoi(args[1]);
			}

			/* Exit the shell program with the specified status */
			exit(exit_status);
		}

		/* Parse command into arguments */
		parse_command(input, args);

		/* Check if command exists in PATH */
		if (!check_command_exists(args[0]))
		{
			continue;
		}

		/* Handle command execution */
		handle_command_execution(args);
	}

	return 0;
}
