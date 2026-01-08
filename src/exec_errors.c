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
void msh_print_last_error(t_shell *sh)
{
    char *op;

    if (sh && sh->last_err_op)
        op = sh->last_err_op;
    if (op && errno)
        fprintf(stderr, "minishell: %s: %s\n", op, strerror(errno));
    else if (op)
        fprintf(stderr, "minishell: %s\n", op);
    else
        fprintf(stderr, "minishell: unknown error\n");
}