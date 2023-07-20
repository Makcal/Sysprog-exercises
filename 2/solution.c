#include "parser.h"
#include "evaluator.h"

int
main(void)
{
    tokenize_command(); // TODO: Implement tokenizer

    char *tokens[] = {"cd", "/", "&&", "ls"};
    list *tokens_list = list_init();

    for (size_t i = 0; i < 4; ++i)
    {
        token *tok = malloc(sizeof(token));
        tok->value = strdup(tokens[i]);

        if (str_equal(tok->value, "|"))
            tok->type = PIPELINE;
        else if (str_equal(tok->value, "&&"))
            tok->type = LOGICAL_OPERATOR_AND;
        else
            tok->type = WORD;

        list_add(tokens_list, tok);
    }

    list_trim(tokens_list);
    int code = evaluate(tokens_list);
    list_free(tokens_list, token_free);
    return code;
}
