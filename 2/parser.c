#include "parser.h"

void
token_free(token *tok)
{
    if (tok->value)
        free(tok->value);

    free(tok);
}

list
*tokenize_command()
{
    return NULL;
}
