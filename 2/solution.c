#include "parser.h"
#include "evaluator.h"
#include <stdio.h>

int
main(void)
{
    for (;;)
    {
        list *tokens = tokenize_command();
        evaluate(tokens);
        list_free(tokens, token_free);

        if (feof(stdin))
            break;
    }

    return EXIT_SUCCESS;
}
