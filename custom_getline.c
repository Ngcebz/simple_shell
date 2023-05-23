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
 * custom_getline - reads input from user
 * @lineptr: buffer to store user input
 * @n: size of the buffer
 * Return: size of the input read
 */
ssize_t custom_getline(char **lineptr, size_t *n)
{
	static char buffer[MAX_INPUT_LENGTH];
	static size_t buffer_pos = 0;
	static ssize_t buffer_size = 0;

	if (lineptr == NULL || n == NULL)
	{
		return -1;
	}

	if (*lineptr == NULL || *n == 0)
	{
		*n = MAX_INPUT_LENGTH;
		*lineptr = malloc(*n);
		if (*lineptr == NULL)
		{
			return -1;
		}
	}

	ssize_t chars_read = 0;
	char *line = *lineptr;
	size_t size = *n;

	while (1)
	{
		if (buffer_pos >= buffer_size)
		{
			buffer_pos = 0;
			buffer_size = read(STDIN_FILENO, buffer, MAX_INPUT_LENGTH);
			if (buffer_size == 0)
			{
				/* End of input */
				if (chars_read == 0)
				{
					return -1;
				}
				else
				{
					break;
				}
			}
			else if (buffer_size == -1)
			{
				return -1;
			}
		}

		char c = buffer[buffer_pos++];
		line[chars_read++] = c;

		if (chars_read == size - 1)
		{
			size *= 2;
			char *new_line = malloc(size);
			if (new_line == NULL)
			{
				return -1;
			}
			for (size_t i = 0; i < chars_read; i++)
			{
				new_line[i] = line[i];
			}
			free(line);
			line = new_line;
		}

		if (c == '\n')
		{
			break;
		}
	}

	line[chars_read] = '\0';
	*lineptr = line;
	*n = size;

	return chars_read;
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
	char *environ[] = {NULL};
	char *input = NULL;
	size_t input_size = 0;

	while (1)
	{
		print_prompt();
		ssize_t read_size = custom_getline(&input, &input_size);

		if (read_size == -1)
		{
			free(input);
			exit(EXIT_SUCCESS);
		}

		execute_command(input);
	}
	return 0;
}
