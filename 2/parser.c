#include "parser.h"
#include <stdio.h>

#define is_word_end(c)  (c) == '&' || (c) == '|' || (c) == '>' || (c) == ' ' || \
                        (c) == '\t' || (c) == '\r' || (c) == '\n' || (c) == EOF

void
token_free(token *tok)
{
    if (!tok)
        return;

    if (tok->value)
        free(tok->value);

    free(tok);
}

static inline char
next_char()
{
    return (char) fgetc(stdin);
}

static token
*init_token(string_builder *chars, int type)
{
    token *tok = malloc(sizeof(token));
    tok->value = to_string(chars);
    tok->type = type;

    return tok;
}

list
*tokenize_command()
{
    char c, next_c;
    list *tokens = list_init();

    bool termination_state = false;
    string_builder *current_token_value = NULL;

    for (; !termination_state;)
    {
        for (c = next_char(); c == ' '; c = next_char());
        if (c == EOF)
            return tokens;

        current_token_value = string_builder_init();

        if (c == '&')
        {
            string_builder_append(current_token_value, c);
            next_c = next_char();

            if (next_c == '&')
            {
                string_builder_append(current_token_value, next_c);
                list_add(tokens, init_token(current_token_value, LOGICAL_OPERATOR_AND));
                string_builder_free(current_token_value);
                current_token_value = NULL;
            } else
            {
                ungetc(next_c, stdin);
                termination_state = true;
                list_add(tokens, init_token(current_token_value, BACKGROUND_EXECUTION));
                string_builder_free(current_token_value);
                current_token_value = NULL;
            }
        } else if (c == '|')
        {
            string_builder_append(current_token_value, c);
            next_c = next_char();

            if (next_c == '|')
            {
                string_builder_append(current_token_value, next_c);
                list_add(tokens, init_token(current_token_value, LOGICAL_OPERATOR_OR));
                string_builder_free(current_token_value);
                current_token_value = NULL;
            } else
            {
                ungetc(next_c, stdin);
                list_add(tokens, init_token(current_token_value, PIPELINE));
                string_builder_free(current_token_value);
                current_token_value = NULL;
            }
        } else if (c == '>')
        {
            string_builder_append(current_token_value, c);
            next_c = next_char();

            if (next_c == '>')
            {
                string_builder_append(current_token_value, next_c);
                list_add(tokens, init_token(current_token_value, OUTPUT_REDIRECTION_APPEND));
                string_builder_free(current_token_value);
                current_token_value = NULL;
            } else
            {
                ungetc(next_c, stdin);
                list_add(tokens, init_token(current_token_value, OUTPUT_REDIRECTION_WRITE));
                string_builder_free(current_token_value);
                current_token_value = NULL;
            }
        } else if (c == '#')
        {
            for (c = next_char(); c != '\n'; c = next_char());
            termination_state = true;
        } else
        {
            if (c == '\n')
                termination_state = true;
            else
            {
                for (;;)
                {
                    if (c == '\'')
                    {
                        for (c = next_char(); c != '\''; c = next_char())
                            string_builder_append(current_token_value, c);

                        c = next_char();
                    }

                    if (c == '"')
                    {
                        for (c = next_char(); c != '"'; c = next_char())
                        {
                            if (c == '\\')
                            {
                                c = next_char();

                                if (c == '"' || c == '\\')
                                    string_builder_append(current_token_value, c);
                                else if (c != '\n')
                                {
                                    string_builder_append(current_token_value, '\\');
                                    string_builder_append(current_token_value, c);
                                }
                            } else
                                string_builder_append(current_token_value, c);
                        }

                        c = next_char();
                    }

                    if (is_word_end(c))
                    {
                        ungetc(c, stdin);
                        break;
                    }

                    if (c == '\\')
                    {
                        c = next_char();

                        if (c == '\n')
                            c = next_char();
                        else
                        {
                            string_builder_append(current_token_value, c);
                            c = next_char();
                        }
                    } else
                    {
                        string_builder_append(current_token_value, c);
                        c = next_char();
                    }
                }

                list_add(tokens, init_token(current_token_value, WORD));
                string_builder_free(current_token_value);
                current_token_value = NULL;
            }
        }
    }

    if (current_token_value)
        string_builder_free(current_token_value);

    list_trim(tokens);
    return tokens;
}
