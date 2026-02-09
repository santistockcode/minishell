/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins_orq.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 17:49:02 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/09 20:11:18 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

// builtins
int		export(t_list **env, char *var);
void	env_unset(t_list **env, char *key);
void	echo_builtin(char **argv);
int		cd_builtin(char **argsv, t_list **env);

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
	(void)p;
	// if (p)
	// 	safe_close_p(p);
	// ACHTUNG: por algún motivo no cerrar p no leakea fds, es correcto.
	safe_close_rd_fds(cmd->redirs);
	safe_close_stage_io(cmd->stage_io);
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
	logger_open_fds("🔥[builtin_orq.c]builtin_stage_exit🔥",
		"[builtin_orq.c]builtin_stage_exit");
	exit(exit_code);
}

/*
 * Check if a command is a builtin.
 * TODO: pwd, exit, env, cd
 */
int	is_builtin(char *cmd)
{
	if (ft_strncmp(cmd, "echo", 4) == 0)
		return (1);
	if (ft_strncmp(cmd, "env", 3) == 0)
		return (1);
	if (ft_strncmp(cmd, "pwd", 3) == 0)
		return (1);
	if (ft_strncmp(cmd, "exit", 4) == 0)
		return (1);
	if (ft_strncmp(cmd, "export", 6) == 0)
		return (1);
	if (ft_strncmp(cmd, "unset", 5) == 0)
		return (1);
	if (ft_strncmp(cmd, "cd", 2) == 0)
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
	if (ft_strncmp(cmd->argv[0], "echo", 5) == 0)
		echo_builtin(cmd->argv);
	if (ft_strncmp(cmd->argv[0], "env", 4) == 0)
		env_builtin(sh->env);
	if (ft_strncmp(cmd->argv[0], "pwd", 4) == 0)
		pwd_builtin();
	if (ft_strncmp(cmd->argv[0], "exit", 5) == 0)
		exit_builtin(cmd->argv, 0, sh->last_status, &sh->should_exit);
	if (ft_strncmp(cmd->argv[0], "export", 7) == 0)
		result = wrap_export(&sh->env, cmd->argv);
	if (ft_strncmp(cmd->argv[0], "unset", 6) == 0)
		unset(&sh->env, cmd->argv);
	if (ft_strncmp(cmd->argv[0], "cd", 3) == 0)
		cd_builtin(cmd->argv, &sh->env);
	return (result);
}
