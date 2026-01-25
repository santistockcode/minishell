#include "../include/minishell.h"

int prepare_redirs(t_list *redirs)
{
    t_redir *redir;

    while (redirs)
    {
        redir = (t_redir *)redirs->content;
        logger("prepare_redirs:REDIR-TARGET", redir->target);
        if (redir && redir->type == R_IN)
        {
            logger("prepare_redirs", "Opening input redirection");
            redir->fd = open_wrap(redir->target, O_RDONLY, 0);
            if (redir->fd == -1)
                return (msh_set_error(NULL, OPEN_OP), -1);
        }
        else if(redir && redir->type == R_OUT_APPEND)
        {
            logger("prepare_redirs", "Opening output append redirection");
            redir->fd = open_wrap(redir->target, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (redir->fd == -1)
                return (msh_set_error(NULL, OPEN_OP), -1);
        }
        else if (redir && redir->type == R_OUT_TRUNC)
        {
            logger("prepare_redirs", "Opening output truncation redirection");
            redir->fd = open_wrap(redir->target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (redir->fd == -1)
                return (msh_set_error(NULL, OPEN_OP), -1);
        }
        redirs = redirs->next;
    }
    return (0);
}
