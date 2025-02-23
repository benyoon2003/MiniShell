#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char input[256];

    // Get the input from stdin
    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        printf("Error reading input.\n");
        return 1;
    }

    // Get rid of the enter character
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n')
    {
        input[len - 1] = '\0';
    }

    char **tokens = tokenizer(input);
    if (tokens == NULL)
    {
        printf("Error tokenizing input.\n");
        return 1;
    }

    // Print all of the tokens from the tokenizer
    for (int i = 0; tokens[i] != NULL; i++)
    {
        printf("%s\n", tokens[i]);
        free(tokens[i]);
    }

    free(tokens);
    return 0;
}
