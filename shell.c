#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>

/**
 * Removes enter from the given string.
 */
void formatString(char *input)
{
    int inputLength = strlen(input);
    if (inputLength > 0 && input[inputLength - 1] == '\n')
    {
        input[inputLength - 1] = '\0';
    }
}

/**
 * Checks if the given string array contains redirection special characters.
 */
int redirection_exists(char **tokenArray)
{
    int redirection_exists = 0;
    for (int i = 0; tokenArray[i] != NULL; i++)
    {
        if (strcmp(tokenArray[i], "<") == 0 || strcmp(tokenArray[i], ">") == 0)
        {
            redirection_exists = 1;
        }
    }
    return redirection_exists;
}

/**
 * Checks if the given string array contains piping special characters.
 */
int piping_exists(char **tokenArray)
{
    int piping_exists = 0;
    for (int i = 0; tokenArray[i] != NULL; i++)
    {
        if (strcmp(tokenArray[i], "|") == 0)
        {
            piping_exists = 1;
        }
    }
    return piping_exists;
}

/**
 * Splits the given array of strings into a 3D array of strings. The splitting is determined
 * by the given string.
 */
char ***split_commands(char **tokenArray, char *splitter)
{
    // Count the number of occurences of the splitting string
    int count = 0;
    for (int i = 0; tokenArray[i] != NULL; i++)
    {
        if (strcmp(tokenArray[i], splitter) == 0)
        {
            count++;
        }
    }

    // Allocated memory for the 3D array of strings
    char ***result = malloc((count + 1) * sizeof(char **));
    if (result == NULL)
    {
        perror("Memory allocation failed");
        exit(1);
    }

    int cmdIndex = 0, tokenIndex = 0;
    result[cmdIndex] = malloc(sizeof(char *) * 256);
    if (result[cmdIndex] == NULL)
    {
        perror("Memory allocation failed");
        exit(1);
    }

    for (int i = 0; tokenArray[i] != NULL; i++)
    {
        // If we detect the splitter null terminate the current array position
        if (strcmp(tokenArray[i], splitter) == 0)
        {
            result[cmdIndex][tokenIndex] = NULL;
            cmdIndex++;

            // Allocate more space for the next array
            result[cmdIndex] = malloc(sizeof(char *) * 256);
            if (result[cmdIndex] == NULL)
            {
                perror("Memory allocation failed");
                exit(1);
            }
            tokenIndex = 0;
        }
        else
        {
            // Allocate memory for strings
            result[cmdIndex][tokenIndex] = malloc(strlen(tokenArray[i]) + 1);
            if (result[cmdIndex][tokenIndex] == NULL)
            {
                perror("Memory allocation failed");
                exit(1);
            }
            strcpy(result[cmdIndex][tokenIndex], tokenArray[i]);
            tokenIndex++;
        }
    }

    // Null terminate dimensions of array
    result[cmdIndex][tokenIndex] = NULL;
    result[cmdIndex + 1] = NULL;

    return result;
}

/**
 * Handle when the user enters sequenced commands denoted by ; between commands
 */
void handle_sequenced_commands(char **tokenArray, int *exitFlag, char *prevCommand)
{
    char ***splitTokens = split_commands(tokenArray, ";");
    for (int i = 0; splitTokens[i] != NULL; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            handle_commands(splitTokens[i], exitFlag, prevCommand);
            exit(0);
        }
        else
        {
            // Wait for each child to finish executing its command
            waitpid(pid, NULL, 0);
        }
    }
    free(splitTokens);
}

/**
 * Handle when the user utilizes redirection (< or >)
 */
void handle_redirection(char **tokenArray, int *exitFlag, char *prevCommand)
{
    int tokenCount = 0;
    int inputRedirection = 0;
    int outputRedirection = 0;
    int indexRedirectonToken = -1;

    // Determine if each array of chars is a input or output redirection
    while (tokenArray[tokenCount] != NULL)
    {
        if (strcmp(tokenArray[tokenCount], ">") == 0)
        {
            outputRedirection = 1;
            indexRedirectonToken = tokenCount;
        }
        else if (strcmp(tokenArray[tokenCount], "<") == 0)
        {
            inputRedirection = 1;
            indexRedirectonToken = tokenCount;
        }
        tokenCount++;
    }

    if (indexRedirectonToken != -1)
    {
        // Allocate memory for the command part of the given token array
        char **commandTokens = malloc(sizeof(char *) * (indexRedirectonToken + 1));
        if (commandTokens == NULL)
        {
            perror("Memory allocation failed");
            exit(1);
        }
        int commandTokenCount = 0;

        // Add just the tokens associated with the command
        while (commandTokenCount != indexRedirectonToken)
        {
            commandTokens[commandTokenCount] = malloc(strlen(tokenArray[commandTokenCount]) + 1);
            if (commandTokens[commandTokenCount] == NULL)
            {
                perror("Memory allocation failed");
                exit(1);
            }
            strcpy(commandTokens[commandTokenCount], tokenArray[commandTokenCount]);
            commandTokenCount++;
        }

        // Allocate memory for the file destination part of the given token array
        char *fileToken = malloc(strlen(tokenArray[indexRedirectonToken + 1]) + 1);
        strcpy(fileToken, tokenArray[indexRedirectonToken + 1]);

        pid_t pid = fork();
        if (pid == 0)
        {
            if (inputRedirection)
            {
                // Replace the stdin file descriptor with the opened file
                int fd = open(fileToken, O_RDONLY);
                if (close(0) == -1)
                {
                    perror("Error closing stdin");
                    exit(1);
                }
                assert(dup(fd) == 0);

                // Close the opened file
                if (close(fd) == -1)
                {
                    perror("Error closing file");
                    exit(1);
                }
            }
            else if (outputRedirection)
            {
                // Replace the stdout with the opened file descriptor
                int fd = open(fileToken, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (close(1) == -1)
                {
                    perror("Error closing stdout");
                    exit(1);
                }
                assert(dup(fd) == 1);

                // Close the opened file
                if (close(fd) == -1)
                {
                    perror("Error closing file");
                    exit(1);
                }
            }
            handle_commands(commandTokens, exitFlag, prevCommand);
            exit(0);
        }
        else
        {
            waitpid(pid, NULL, 0);
        }

        // Free allocated memory
        for (int i = 0; commandTokens[i] != NULL; i++)
        {
            free(commandTokens[i]);
        }
        free(commandTokens);
        free(fileToken);
    }
}

/**
 * Handle piping so that command inputs and outputs are redirected from left to right
 */
void handle_piping(char **tokenArray, int *exitFlag, char *prevCommand)
{
    char ***splitTokens = split_commands(tokenArray, "|");

    // Count the number of split commands
    int numCommands = 0;
    while (splitTokens[numCommands] != NULL)
    {
        numCommands++;
    }

    // Create an array of pipes for each piping between commands
    int pipes[numCommands - 1][2];

    for (int i = 0; i < numCommands - 1; i++)
    {
        assert(pipe(pipes[i]) == 0);
    }

    for (int i = 0; i < numCommands; i++)
    {
        // Fork for each command
        pid_t pid = fork();
        if (pid == 0)
        {

            // Close the standard input if we have at least one command
            if (i > 0)
            {
                close(0);
                dup(pipes[i - 1][0]);
            }

            // Close the standard output if there is still at least one command at the end of the piping
            if (i < numCommands - 1)
            {
                close(1);
                dup(pipes[i][1]);
            }

            // Close all of the previous pipes we already went through
            for (int j = 0; j < numCommands - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            handle_commands(splitTokens[i], exitFlag, prevCommand);
            exit(0);
        }
    }

    for (int i = 0; i < numCommands - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Need to wait for all of the child processes to finish
    for (int i = 0; i < numCommands; i++)
    {
        wait(NULL);
    }
    free(splitTokens);
}

/**
 * Handles all built in and non built in commands
 */
void handle_commands(char **tokenArray, int *exitFlag, char *prevCommand)
{
    if (redirection_exists(tokenArray))
    {
        handle_redirection(tokenArray, exitFlag, prevCommand);
    }
    if (piping_exists(tokenArray))
    {
        handle_piping(tokenArray, exitFlag, prevCommand);
    }

    // Only handle simple commands if we checked that redirection and piping doesn't exist here
    if (!redirection_exists(tokenArray) && !piping_exists(tokenArray))
    {

        // Enters a new shell
        if (strcmp("./shell", tokenArray[0]) == 0)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                enter_new_shell();
            }
            else
            {
                waitpid(pid, NULL, 0);
            }
        }

        // Exits the current shell
        else if (strcmp("exit", tokenArray[0]) == 0)
        {
            *exitFlag = 1;
        }

        // Changes directory to the given path
        else if (strcmp("cd", tokenArray[0]) == 0)
        {
            if (tokenArray[1] != NULL)
            {
                if (chdir(tokenArray[1]) == -1)
                {
                    perror("Error - cd failed");
                }
            }
            else
            {
                printf("cd: missing argument\n");
            }
        }

        // Executes the script located at the given path
        else if (strcmp("source", tokenArray[0]) == 0)
        {
            if (tokenArray[1] != NULL)
            {
                FILE *file = fopen(tokenArray[1], "r");
                if (!file)
                {
                    perror("Error opening file");
                    return;
                }
                char line[256];

                // Execute every line of the given file
                while (fgets(line, sizeof(line), file) != NULL)
                {
                    
                    // Make sure to get rid of the enter after each line
                    formatString(line);
                    if (line != NULL)
                    {
                        char **fileTokenArray = tokenizer(line);
                        handle_commands(fileTokenArray, exitFlag, prevCommand);
                        free(fileTokenArray);
                    }
                }
                fclose(file);
            }
            else
            {
                printf("Can't find the file\n");
            }
        }

        // Prints the previous command line and executes it again
        else if (strcmp("prev", tokenArray[0]) == 0)
        {
            if (prevCommand != NULL)
            {
                char **prevTokenArray = tokenizer(prevCommand);
                handle_commands(prevTokenArray, exitFlag, prevCommand);
                free(prevTokenArray);
            }
            else
            {
                printf("No previous command");
            }
        }

        // Explains all built in commands in the shell
        else if (strcmp("help", tokenArray[0]) == 0)
        {
            printf("help: Explains all the built-in commands available in the mini-shell\n");
            printf("cd <PATH>: Change directory to the specified path.\n");
            printf("source <FILENAME>: Execute commands from a file.\n");
            printf("prev: Execute the previous command again.\n");
            printf("exit: Exit the shell.\n");
        }

        // Executes all non built in commands 
        else
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                if (execvp(tokenArray[0], tokenArray) == -1)
                {
                    perror("Error - execvp failed");
                    exit(1);
                }
            }
            else
            {
                waitpid(pid, NULL, 0);
            }
        }
    }
}

/**
 * Starts a new shell with the welcome message and bash prompts
 */
void enter_new_shell()
{
    char userInput[256];
    char prevCommand[256] = "";
    int exitFlag = 0;

    // Make sure to exit the while loop if the user chooses to exit
    while (!exitFlag)
    {
        printf("shell $ ");
        if (fgets(userInput, sizeof(userInput), stdin) == NULL)
        {
            printf("\n");
            break;
        }
        formatString(userInput);

        if (strlen(userInput) == 0)
        {
            continue;
        }

        // Can't get a prev command if we haven't executed any other commands yet
        if (strcmp(userInput, "prev") != 0)
        {
            strcpy(prevCommand, userInput);
        }
        else {
            printf("No command history\n");
        }

        char **tokenArray = tokenizer(userInput);
        handle_sequenced_commands(tokenArray, &exitFlag, prevCommand);
        free(tokenArray);
    }

    printf("Bye bye.\n");
    exit(0);
}

int main(int argc, char **argv)
{
    printf("Welcome to mini-shell.\n");
    enter_new_shell();
    return 0;
}
