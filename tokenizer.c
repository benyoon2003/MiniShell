#include "tokenizer.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reads sequences of numbers, letters, and _
 */
static int read_word_string(const char *input, char *output)
{
    int i = 0;

    // Loop to check if current character is either a letter, number, or _
    while (input[i] != '\0' && (isalpha(input[i]) || isdigit(input[i]) || input[i] == '_' || input[i] == '-' || input[i] == '.' || input[i] == '/'))
    {
        output[i] = input[i];
        i++;
    }
    output[i] = '\0';
    return i;
}

/**
 * Reads a quote-enclosed string.
 */
static int read_quote_enclosed_string(const char *input, char *output)
{
    int i = 1, j = 0;

    // We should add all of the chars within the given string until the string ends or we reach the closing quote
    while (input[i] != '\0' && input[i] != '"')
    {
        output[j] = input[i];
        j++;
        i++;
    }

    // Need to skip the quote for next token
    if (input[i] == '"')
    {
        i++;
    }

    output[j] = '\0';
    return i;
}

/**
 * Tokenizes the given string into basic words, special characters, and words enclosed in special characters
 */
char **tokenizer(char *argv)
{
    char buf[256];
    char **tokens = malloc(256 * sizeof(char *));
    if (tokens == NULL)
    {
        printf("Memory allocation failed for tokens\n");
        return NULL;
    }

    // Initialize token index and character index for given string
    int tokenCount = 0;
    int i = 0;

    while (argv[i] != '\0')
    {

        // Checks if the current char is the beginning of a word
        if (isalpha(argv[i]) || isdigit(argv[i]) || argv[i] == '_' || argv[i] == '-' || argv[i] == '.' || argv[i] == '/')
        {

            // Calls the function to start from the current position of the string
            // since we already know it is the start of a word
            i += read_word_string(&argv[i], buf);
        }
        else
        {
            switch (argv[i])
            {
            // We can handle all of these special characters as individual tokens
            case '<':
            case '>':
            case ';':
            case '|':
            case '(':
            case ')':
                buf[0] = argv[i];
                buf[1] = '\0';
                i++;
                break;
            case '"':
                i += read_quote_enclosed_string(&argv[i], buf);
                break;
            // Empty spaces are irrelevant if not enclosed by quotes
            case ' ':
                i++;
                continue;
            // Error out if we can't handle the character
            default:
                printf("Unknown special character '%c'.\n", argv[i]);
                i++;
                continue;
            }
        }
        // Allocate memory to element of tokens and copy from buffer
        tokens[tokenCount] = malloc(strlen(buf) + 1);
        if (tokens[tokenCount] == NULL)
        {
            printf("Failed to allocate memory to tokens element\n");
            return NULL;
        }
        strcpy(tokens[tokenCount], buf);
        tokenCount++;
    }

    tokens[tokenCount] = NULL;
    return tokens;
}
