/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fds_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 18:04:14 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/01 10:16:26 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

// exec stage utils 2
void	stage_exit_print(t_shell *sh, t_cmd *cmd, int *p, int exit_code);

void	dup2_stage_io(t_shell *sh, t_cmd *cmd, int *p)
{
	const t_stage_io	*rdr_spec;

	rdr_spec = cmd->stage_io;
	if (rdr_spec && rdr_spec->in_fd != -1)
	{
		if (dup2_wrap(rdr_spec->in_fd, STDIN_FILENO) == -1)
		{
			msh_set_error(sh, DUP2_OP);
			stage_exit_print(sh, cmd, p, EXIT_FAILURE);
		}
		safe_close(rdr_spec->in_fd);
	}
	if (rdr_spec && rdr_spec->out_fd != -1)
	{
		if (dup2_wrap(rdr_spec->out_fd, STDOUT_FILENO) == -1)
		{
			msh_set_error(sh, DUP2_OP);
			stage_exit_print(sh, cmd, p, EXIT_FAILURE);
		}
		safe_close(rdr_spec->out_fd);
	}
}

int	require_standard_fds(t_shell *sh)
{
	struct stat	*statbuf;
	int			fd;

	fd = 0;
	statbuf = malloc(sizeof(struct stat));
	if (!statbuf)
		return (msh_set_error(sh, MALLOC_OP), -1);
	while (fd <= 2)
	{
		if (fstat(fd, statbuf) == -1)
		{
			msh_set_error(sh, MISSING_FDS_OP);
			return (-1);
		}
		fd++;
	}
	free(statbuf);
	return (0);
}

void	safe_close_rd_fds(t_list *redirs)
{
	t_redir	*redir;

	while (redirs)
	{
		redir = (t_redir *)redirs->content;
		if (redir)
		{
			if (safe_close(redir->fd) == -1)
			{
				msh_set_error(NULL, CLOSE_OP);
			}
		}
		redirs = redirs->next;
	}
}

int	msh_save_fds(int *save_in, int *save_out, int *save_err)
{
	*save_in = dup_wrap(STDIN_FILENO);
	*save_out = dup_wrap(STDOUT_FILENO);
	*save_err = dup_wrap(STDERR_FILENO);
	if (*save_in == -1 || *save_out == -1 || *save_err == -1)
	{
		if (*save_in != -1)
			close(*save_in);
		if (*save_out != -1)
			close(*save_out);
		if (*save_err != -1)
			close(*save_err);
		return (-1);
	}
	return (0);
}

void	msh_restore_fds(int save_in, int save_out, int save_err)
{
	if (save_in != -1)
	{
		dup2(save_in, STDIN_FILENO);
		close(save_in);
	}
	if (save_out != -1)
	{
		dup2(save_out, STDOUT_FILENO);
		close(save_out);
	}
	if (save_err != -1)
	{
		dup2(save_err, STDERR_FILENO);
		close(save_err);
	}
}
