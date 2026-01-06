#include "../Libft/include/libft.h"
#include "../include/exec.h"
#include "../include/log.h"
#include "../include/syswrap.h"
#include <stdio.h>
#include <stdlib.h>

void	unlink_hds(t_list *cmds)
{
	t_list	*cmd_node;
	t_cmd	*cmd;
	t_list	*redir_node;
	t_redir	*redir;

	MSH_LOG("Unlinking here_docs associated with commands");
	cmd_node = cmds;
	while (cmd_node)
	{
		cmd = (t_cmd *)cmd_node->content;
		redir_node = cmd->redirs;
		while (redir_node)
		{
			redir = (t_redir *)redir_node->content;
			if (redir && redir->type == R_HEREDOC)
			{
				MSH_LOG("Unlinking here_doc file: %s", redir->target);
				if (redir->target)
					unlink_wrap(redir->target);
			}
			redir_node = redir_node->next;
		}
		cmd_node = cmd_node->next;
	}
}
