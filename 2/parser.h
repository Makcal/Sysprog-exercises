#include "command.h"
#include "utils.h"

#define PIPELINE "|"
#define BACKGROUND_EXECUTION "&"

#define AND_OPERATOR "&&"
#define OR_OPERATOR "||"

#define STDOUT_REDIRECTION ">"
#define STDOUT_REDIRECTION_APPEND ">>"

#define is_redirection(str) str_equal(str, STDOUT_REDIRECTION) || \
                            str_equal(str, STDOUT_REDIRECTION_APPEND)

#define is_appending(str)   str_equal(str, STDOUT_REDIRECTION_APPEND)

#define is_logical(str)     str_equal(str, AND_OPERATOR) || str_equal(str, OR_OPERATOR)
#define is_pipeline(str)    str_equal(str, PIPELINE)
#define is_background(str)  str_equal(str, BACKGROUND_EXECUTION)

#define is_special(str)     is_redirection(str) || is_pipeline(str) || \
                            is_background(str)  || is_logical(str)

list
*parse(char *input_line);
