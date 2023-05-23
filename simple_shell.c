#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "main.h"

#define MAX_INPUT_LENGTH 1024

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
		system(command);
		_exit(EXIT_SUCCESS);
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
 * read_commands_from_file - reads commands from a file
 * @filename: name of the file to read
 */
void read_commands_from_file(const char *filename)
{
	FILE *file = fopen(filename, "r");
	char line[MAX_INPUT_LENGTH];

	if (file == NULL)
	{
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	while (fgets(line, sizeof(line), file) != NULL)
	{
		line[strcspn(line, "\n")] = '\0'; // Remove newline character
		execute_command(line);
	}

	fclose(file);
}

/**
 * main - shell program that reads and executes
 * simple one-word commands from the user or a file.
 * @argc: argument count
 * @argv: argument vector
 * Return: 0 on success, non-zero on failure
 */
int main(int argc, char **argv)
{
	if (argc > 1)
	{
		/* Run commands from file */
		read_commands_from_file(argv[1]);
	}
	else
	{
		/* Interactive mode */
		char input[MAX_INPUT_LENGTH];

		while (1)
		{
			printf("$ ");
			fflush(stdout);

			if (fgets(input, sizeof(input), stdin) == NULL)
				break;

			input[strcspn(input, "\n")] = '\0'; // Remove newline character

			execute_command(input);
		}
	}

	return 0;
}
