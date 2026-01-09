#include "../include/minishell.h"

void msh_set_error(t_shell *sh, const char *op)
{
    char *tmp_op;

    if (!sh)
        return;
    free(sh->last_err_op);
    tmp_op = ft_strdup(op);
    if (!tmp_op)
        sh->last_err_op = NULL;
    else
        sh->last_err_op = tmp_op;
}

// top level printing helper
/*
When a syscall fails, it often sets errno. Except that we are still
using ft_strdup above in msh_set_error, which may as well fail. 
That's the reason to have a fallback "unknown error/internal error" message.
*/
void msh_print_last_error(t_shell *sh)
{
    char *op = NULL;

    if (sh && sh->last_err_op)
        op = sh->last_err_op;

    if (op && errno)
        fprintf(stderr, "minishell: %s: %s\n", op, strerror(errno));
    else if (op)
        fprintf(stderr, "minishell: %s\n", op);
    else if (errno)
        fprintf(stderr, "minishell: %s\n", strerror(errno));
    else
        fprintf(stderr, "minishell: unknown error/internal error\n");

    if (sh && sh->last_err_op)
    {
        free(sh->last_err_op);
        sh->last_err_op = NULL;
    }
}