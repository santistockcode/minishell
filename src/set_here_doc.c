#include "../Libft/include/libft.h"
#include "../include/exec.h"
#include "../include/log.h"
#include <stdio.h>
#include <stdlib.h>

/*
This functions iterates over commands and if finds one with redir 
R_HEREDOC, it will fetch the heredoc content from the user and set it as the target.

Doesn't pass norminette but it will depend largely on how do I manage errors
*/
int	set_here_doc(t_shell *sh, t_list *cmd_first)
{
	t_list	*cmd_list;
	t_cmd	*current_cmd;
	t_list	*redir_list;
	t_redir	*redir;
	int		suffix;

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
				MSH_LOG("Heredoc specified for cmd number %d", suffix);
				if (fetch_hd_from_user(sh, &(redir->target),
						!(redir->quoted), suffix++) == -1)
					return (-1);
				break ;
			}
			redir_list = redir_list->next;
		}
		cmd_list = cmd_list->next;
	}
	return (0);
}
