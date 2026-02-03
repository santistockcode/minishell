/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins_orq.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 17:49:02 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/03 08:28:14 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

// builtins
int		export(t_list **env, char *var);
void	env_unset(t_list **env, char *key);
void	echo(char **argv);

// exit utils
void	safe_close_p(int *p);
void	safe_close_stage_io(t_stage_io *stage_io);
void	free_shell_child(t_shell *sh);

// fds utils
void	safe_close_rd_fds(t_list *redirs);
void	dup2_stage_io(t_shell *sh, t_cmd *cmd, int *p);

// free cmds
void	free_cmd_struct(void *input);

// clean everything when child process ends successfully
void	builtin_stage_exit(t_shell *sh, t_cmd *cmd, int *p, int exit_code)
{
	if (p)
		safe_close_p(p);
	safe_close_rd_fds(cmd->redirs);
	// safe_close_stage_io(cmd->stage_io);
	free(cmd->stage_io);
	msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
	safe_close(sh->save_in);
	safe_close(sh->save_out);
	safe_close(sh->save_err);
	if (sh->cmds_start)
	{
		free_cmds(sh->cmds_start);
	}
	else
		free_cmd_struct(cmd);
	free_shell_child(sh);
	logger_open_fds( "🔥[builtin_orq.c]builtin_stage_exit🔥", "[builtin_orq.c]builtin_stage_exit");
	exit(exit_code & 0xff);
}

/*
 * Check if a command is a builtin.
 * TODO: pwd, exit, env, cd
 */
int	is_builtin(char *cmd)
{
	if (ft_strncmp(cmd, "echo", 4) == 0)
		return (1);
	if (ft_strncmp(cmd, "export", 6) == 0)
		return (1);
	if (ft_strncmp(cmd, "unset", 5) == 0)
		return (1);
	return (0);
}

// TODO: better to treat each builtin as independant programm
// every error has been set, every error has been printed
// here we just pass the final status
int	exec_builtin(t_cmd *cmd, t_shell *sh)
{
	int	result;

	result = 0;
	if (ft_strncmp(cmd->argv[0], "echo", 4) == 0)
		echo(cmd->argv);
	if (ft_strncmp(cmd->argv[0], "export", 6) == 0)
		result = export(&sh->env, cmd->argv[1]);
	if (ft_strncmp(cmd->argv[0], "unset", 5) == 0)
		env_unset(&sh->env, cmd->argv[1]);
	return (result);
}
