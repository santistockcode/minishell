#include "../Libft/include/libft.h"
#include "../include/exec.h"
#include "../include/log.h"
#include <stdio.h>
#include <stdlib.h>

int	set_here_doc(t_shell *sh, t_list *cmd_first)
{
	t_list	*cmd_list;
	t_cmd	*current_cmd;
	t_list	*redir_list;
	t_redir	*redir;
	int		suffix;

	MSH_LOG("Looking for here_doc and setting target if found");
	cmd_list = cmd_first;
	suffix = 0;
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
				free(redir->target);
				redir->target = fetch_hd_from_user(sh, redir->target, !(redir->quoted), suffix++);
				break ;
			}
			else
				MSH_LOG("No heredoc redirection found in this redir");
			redir_list = redir_list->next;
		}
		cmd_list = cmd_list->next;
	}
	// in case of error returns (-1) and frees everything if that's the case
	return (0);
}
