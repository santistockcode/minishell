/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   convert_to_exec.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 12:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 23:38:13 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"
#include "../../include/set_exec.h"

static int	convert_and_add_cmd(t_shell *shell, t_command *command)
{
	t_cmd	*exec_cmd;
	t_list	*exec_node;

	exec_cmd = convert_command_to_cmd(command);
	if (!exec_cmd)
	{
		free_cmds(shell->exec_cmds);
		shell->exec_cmds = NULL;
		return (MALLOC_ERROR);
	}
	exec_node = ft_lstnew(exec_cmd);
	if (!exec_node)
	{
		free_cmds(shell->exec_cmds);
		shell->exec_cmds = NULL;
		return (MALLOC_ERROR);
	}
	ft_lstadd_back(&shell->exec_cmds, exec_node);
	return (SUCCESS);
}

int	set_to_exec(t_shell *shell)
{
	t_list		*cmd_node;
	t_command	*command;
	int			status;

	if (!shell || !shell->cmds_start)
		return (INPUT_ERROR);
	shell->exec_cmds = NULL;
	cmd_node = shell->cmds_start;
	while (cmd_node)
	{
		command = (t_command *)cmd_node->content;
		status = convert_and_add_cmd(shell, command);
		if (status != SUCCESS)
			return (status);
		cmd_node = cmd_node->next;
	}
	logger_exec_cmds(shell->exec_cmds, "after set_to_exec");
	return (SUCCESS);
}

/**
 * convert_command_to_cmd() - Convert single t_command to t_cmd
 *
 * Creates a t_cmd structure with:
 * - argv: array of command and arguments
 * - redirs: list of redirections
 */
t_cmd	*convert_command_to_cmd(t_command *command)
{
	t_cmd	*cmd;

	if (!command)
		return (NULL);
	cmd = ft_calloc(1, sizeof(t_cmd));
	if (!cmd)
		return (NULL);
	cmd->argv = build_argv(command);
	if (!cmd->argv)
	{
		free(cmd);
		return (NULL);
	}
	cmd->redirs = build_redirs(command);
	return (cmd);
}

/**
 * convert_io_type() - Convert parsing redirect type to exec redirect type
 *
 * Maps between the two enumeration types:
 * REDIR_IN -> R_IN
 * REDIR_OUT -> R_OUT_TRUNC
 * REDIR_APPEND -> R_OUT_APPEND
 * HEREDOC -> R_HEREDOC
 */
t_redir_type	convert_io_type(t_io_redirect_type io_type)
{
	if (io_type == REDIR_IN)
		return (R_IN);
	else if (io_type == REDIR_OUT)
		return (R_OUT_TRUNC);
	else if (io_type == REDIR_APPEND)
		return (R_OUT_APPEND);
	else if (io_type == HEREDOC)
		return (R_HEREDOC);
	return (R_IN);
}
