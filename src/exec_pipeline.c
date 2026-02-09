/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_pipeline.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 17:59:34 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/09 21:25:24 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

// fds_utils.c
void	safe_close_rd_fds(t_list *redirs);
int		msh_save_fds(int *save_in, int *save_out, int *save_err);
void	msh_restore_fds(int save_in, int save_out, int save_err);
int		require_standard_fds(t_shell *sh);

// exit utils
void	safe_close_p(int *p);

// signals
void	setup_signals_ignore(void);

/* EXEC_PIPELINE */

/*
Not checkign if nstages < 1 because of norminette.
Do not call run_pipeline with incorrect cmd_first.
Returns -1 on fork error (ya sea en first, middle or last)
*/
int	run_pipeline(t_shell *sh, t_list *cmd_first, int nstages, pid_t *pid)
{
	int		p[2];
	int		in_fd;
	t_list	*current_cmd_node;

	sh->cmds_start = cmd_first;
	if (pipe(p) == -1)
		return (msh_set_error(sh, PIPE_OP), -1);
	current_cmd_node = cmd_first;
	if (do_first_command(sh, (t_cmd *)current_cmd_node->content, p) == -1)
		return (safe_close_p(p), -1);
	in_fd = p[0];
	current_cmd_node = current_cmd_node->next;
	while (nstages-- > 2)
	{
		if (pipe(p) == -1)
			return (safe_close_p(p), safe_close(in_fd), msh_set_error(sh,
					PIPE_OP), -1);
		if (do_middle_commands(sh, (t_cmd *)current_cmd_node->content, p,
				in_fd) == -1)
			return (-1);
		in_fd = p[0];
		current_cmd_node = current_cmd_node->next;
	}
	return (do_last_command(sh, (t_cmd *)current_cmd_node->content, in_fd,
			pid));
}

int	wait_last(int status)
{
	int	last_status;

	last_status = 0;
	if (WIFSIGNALED(status))
	{
		last_status = 128 + WTERMSIG(status);
		if (WTERMSIG(status) == SIGINT)
			write(STDOUT_FILENO, "\n", 1);
		else if (WTERMSIG(status) == SIGQUIT)
			write(STDERR_FILENO, "Quit (core dumped)\n", 19);
	}
	else if (WIFEXITED(status))
		last_status = WEXITSTATUS(status);
	return (last_status);
}

int	wait_all_pipeline(pid_t last_pid, int nstages)
{
	int		status;
	int		last_status;
	pid_t	pid;
	int		reaped;

	last_status = 0;
	reaped = 0;
	setup_signals_ignore();
	while (reaped < nstages)
	{
		pid = waitpid(-1, &status, 0);
		if (pid == -1)
			break ;
		reaped++;
		if (pid == last_pid)
		{
			last_status = wait_last(status);
		}
	}
	return (setup_signal(), last_status);
}

// returns -1 on fork error happened before last command on parent
// it interrupted pipeline: FATAL!
int	msh_exec_pipeline(t_shell *sh, t_list *cmd_first, int nstages)
{
	int		result;
	pid_t	pid;

	if (require_standard_fds(sh) == -1)
		return (-1);
	result = run_pipeline(sh, cmd_first, nstages, &pid);
	sh->cmds_start = NULL;
	if (result == -1)
		return (-1);
	return (wait_all_pipeline(pid, nstages));
}

// int		wtpd_resp;
// int		status;
// setup_signals_ignore();
// wtpd_resp = waitpid(pid, &status, 0);
// if (wtpd_resp == -1)
// 	return (msh_set_error(sh, WAITPID_OP), -1);
// if (WIFSIGNALED(status))
// {
// 	result = 128 + WTERMSIG(status);
// 	if (WTERMSIG(status) == SIGINT)
// 		write(STDOUT_FILENO, "\n", 1);
// 	else if (WTERMSIG(status) == SIGQUIT)
// 		write(STDOUT_FILENO, "Quit\n", 5);
// }
// else if (WIFEXITED(status))
// 	result = WEXITSTATUS(status);
// setup_signal();
// return (result);