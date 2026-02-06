/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_cmds.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 18:04:34 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/06 16:42:20 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

// free cmds
void	free_envp(const char **envp)
{
	const char	**original;

	if (!envp)
		return ;
	original = envp;
	while (*envp)
	{
		free((char *)*envp);
		envp++;
	}
	free((char **)original);
}

static void	free_redir(void *redir_ptr)
{
	t_redir	*redir;

	redir = (t_redir *)redir_ptr;
	if (!redir)
		return ;
	if (redir->target)
		free(redir->target);
	free(redir);
}

void	free_cmd_struct(void *input)
{
	t_cmd	*cmd;
	char	**argv;

	cmd = (t_cmd *)input;
	if (!cmd)
		return ;
	if (cmd->argv)
	{
		argv = cmd->argv;
		while (*argv)
		{
			free(*argv);
			argv++;
		}
		free(cmd->argv);
		cmd->argv = NULL;
	}
	if (cmd->redirs)
		ft_lstclear(&cmd->redirs, free_redir);
	free(cmd);
}

void	free_cmds(t_list *cmd_first)
{
	t_list	*node;

	node = cmd_first;
	ft_lstclear(&node, &free_cmd_struct);
}
