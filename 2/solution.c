#include "parser.h"
#include "evaluator.h"

int
main(void)
{
    parse(NULL); // TODO: Implement tokenizer

    char *tokens[] = {"ls", "/", "|", "grep", "lib", "&&", "echo", "aaa", "||", "echo", "bb", "&"};

    list *tokens_list = list_init();

    for (size_t i = 0; i < 12; ++i)
        list_add(tokens_list, tokens[i]);

    list_trim(tokens_list);

    int code = evaluate(tokens_list);
    return code;
}
