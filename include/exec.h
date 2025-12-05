#ifndef EXEC_H
# define EXEC_H
#include "minishell.h"

typedef enum e_redir_type {
    R_IN,          // <
    R_OUT_TRUNC,   // >
    R_OUT_APPEND,  // >>
    R_HEREDOC      // <<
}   t_redir_type;

typedef struct s_redir {
    t_redir_type      type;
    int               fd;            // usually 0 or 1
    char             *target;        // filename or heredoc delimiter
    int               delim_quoted;  // for heredoc; 1 if delimiter was quoted
}   t_redir;

// one simple command in a pipeline
typedef struct s_cmd {
    char           **argv;        // NULL-terminated; raw words (already de-quoted)
    t_list        *redirs;       // linked list, or NULL
}   t_cmd;


// "backend"
int    exec_pipeline(t_shell *sh, t_list *cmd_first);
void   free_pipeline(t_list *cmd_first);



#endif