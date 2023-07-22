#include "parser.h"
#include "evaluator.h"
#include <stdio.h>

//#include "../utils/heap_help/heap_help.h"

int
main(void)
{
    exit_context context;
    int exit_code = EXIT_SUCCESS;

    while (!feof(stdin))
    {
        list *tokens = tokenize_command();
        context = evaluate(tokens);

        if (context.exit_code != EXIT_NO_CODE)
            exit_code = context.exit_code;

        list_free(tokens, token_free);

        if (context.is_terminate)
            break;
    }

//    heaph_get_alloc_count();
    return exit_code;
}
