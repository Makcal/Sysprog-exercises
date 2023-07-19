#include "command.h"
#include "utils.h"

enum
{
    WORD = 01000000,
    SPECIAL = 00111111,

    BACKGROUND_EXECUTION = 00100000,
    PIPELINE = 00010000,

    LOGICAL_OPERATOR = 00001100,
    LOGICAL_OPERATOR_AND = 00000100,
    LOGICAL_OPERATOR_OR = 00001000,

    OUTPUT_REDIRECTION = 00000011,
    OUTPUT_REDIRECTION_WRITE = 00000001,
    OUTPUT_REDIRECTION_APPEND = 00000010,
};

#define is_logical(type)                ((type) & LOGICAL_OPERATOR)
#define is_logical_and(type)            ((type) == LOGICAL_OPERATOR_AND)
#define is_logical_or(type)             ((type) == LOGICAL_OPERATOR_OR)

#define is_redirection(type)            ((type) & OUTPUT_REDIRECTION)
#define is_redirection_write(type)      ((type) == OUTPUT_REDIRECTION_WRITE)
#define is_redirection_append(type)     ((type) == OUTPUT_REDIRECTION_APPEND)

#define is_pipeline(type)               ((type) == PIPELINE)
#define is_background_execution(type)   ((type) == BACKGROUND_EXECUTION)

#define is_special(type)                ((type) & SPECIAL)
#define is_word(type)                   ((type) == WORD)


typedef struct
{
    char *value;
    int type;
} token;

list
*parse(const char *input_line);
