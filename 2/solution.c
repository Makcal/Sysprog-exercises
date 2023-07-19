#include "parser.h"
#include "evaluator.h"

int
main(void)
{
    parse(NULL); // TODO: Implement tokenizer

    char *tokens[] = {"ls", "-a", "/"};

    list *tokens_list = list_init();

    for (size_t i = 0; i < 3; ++i)
    {
        token *tok = malloc(sizeof(token));
        tok->value = strdup(tokens[i]);
        tok->type = WORD;

        list_add(tokens_list, tok);
    }

    list_trim(tokens_list);

    int code = evaluate(tokens_list);
    list_free(tokens_list, free);
    return code;
}
