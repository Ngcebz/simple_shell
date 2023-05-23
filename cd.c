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
 * change_directory - changes the current directory
 * @path: path of the directory to change to
 * @prev_dir: previous directory
 * Return: 0 on success, -1 on failure
 */
int change_directory(const char *path, char *prev_dir)
{
	char *home_dir;
	char *new_dir;

	if (path == NULL)
	{
		home_dir = getenv("HOME");
		if (home_dir == NULL)
		{
			write(STDERR_FILENO, "cd: No home directory\n", 22);
			return -1;
		}
		new_dir = home_dir;
	}
	else if (strcmp(path, "-") == 0)
	{
		if (prev_dir == NULL)
		{
			write(STDERR_FILENO, "cd: No previous directory\n", 26);
			return -1;
		}
		new_dir = prev_dir;
	}
	else
	{
		new_dir = path;
	}

	if (chdir(new_dir) != 0)
	{
		perror("cd");
		return -1;
	}

	char *cwd = getcwd(NULL, 0);
	if (cwd == NULL)
	{
		perror("getcwd");
		return -1;
	}
	if (setenv("PWD", cwd, 1) != 0)
	{
		perror("setenv");
		free(cwd);
		return -1;
	}

	free(cwd);

	return 0;
}

/**
 * handle_builtin_commands - handle builtin commands
 * @args: command arguments
 * @prev_dir: pointer to previous directory
 * Return: 1 if command is builtin, 0 otherwise
 */
int handle_builtin_commands(char **args, char *prev_dir)
{
	if (strcmp(args[0], "cd") == 0)
	{
		if (args[1] != NULL)
		{
			if (change_directory(args[1], prev_dir) != 0)
				return 0;
		}
		else
		{
			if (change_directory(NULL, prev_dir) != 0)
				return 0;
		}
		return 1;
	}
	else if (strcmp(args[0], "setenv") == 0)
	{
		/* implementation for setenv command */
		return 1;
	}
	else if (strcmp(args[0], "unsetenv") == 0)
	{
		/* implementation for unsetenv command */
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
	char *prev_dir = NULL;

	while (1)
	{
		print_prompt();
		ssize_t read_size = read_input(input);

		if (read_size == 0)
			continue;

		if (handle_builtin_commands(args, prev_dir) == 0)
			execute_command(args[0]);
	}

	return (0);
}
