#include "../include/minishell.h"

void	unlink_hds(t_list *cmds)
{
	t_list	*cmd_node;
	t_cmd	*cmd;
	t_list	*redir_node;
	t_redir	*redir;

	logger("unlink_hds", "Unlinking here_docs associated with commands");
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
				logger("unlink_hds", "Unlinking here_doc file");
				if (redir->target)
					unlink_wrap(redir->target);
			}
			redir_node = redir_node->next;
		}
		cmd_node = cmd_node->next;
	}
}
