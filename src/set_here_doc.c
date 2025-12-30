#include "../include/exec.h"
#include "../include/log.h"
#include "../Libft/include/libft.h"
#include <stdio.h>
#include <stdlib.h>

int    set_here_doc(t_shell *sh, t_list *cmd_first)
{
    MSH_LOG("Looking for here_doc and setting target if found");

    t_list *cmd_list;
    t_cmd *current_cmd;
    t_list *redir_list;
    t_redir *redir;
    
    cmd_list = cmd_first;
    while (cmd_list)
    {
        current_cmd = (t_cmd *)cmd_list->content;
        redir_list = current_cmd->redirs;
        while (redir_list)
        {
            redir = (t_redir *)redir_list->content;
            if (redir && redir->type == R_HEREDOC)
            {
                MSH_LOG("Found heredoc redirection");
                fetch_here_doc_from_user(sh, redir->target, redir->should_expand);
            }
            redir_list = redir_list->next;
        }
        cmd_list = cmd_list->next;
    }

    // in case of error returns (-1) and frees everything if that's the case

    return (0);
}
