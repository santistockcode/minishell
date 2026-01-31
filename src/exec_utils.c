/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 12:19:35 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/31 18:03:23 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

extern volatile sig_atomic_t	g_exit_status;

// exit_utils.c
void							stage_exit_print(t_shell *sh, t_cmd *cmd,
									int *p, int exit_code);

void	exit_from_no_path(t_shell *sh, t_cmd *cmd, int *p)
{
	if (sh->last_err_op && ft_strncmp(sh->last_err_op, MALLOC_OP,
			ft_strlen(sh->last_err_op)) == 0)
		stage_exit_print(sh, cmd, p, EXIT_FAILURE);
	if (sh->last_errno == EACCES)
		stage_exit_print(sh, cmd, p, 126);
	msh_set_error(sh, cmd->argv[0]);
	stage_exit_print(sh, cmd, p, 127);
}

/*
Protects against closing invalid file descriptors.
Dosn't interrupt flow but logs an error message.
*/
int	safe_close(int fd)
{
	if (fd > 0)
	{
		return (close(fd));
	}
	return (0);
}

/*
This function returns a unique process ID for the current shell.
*/
int	get_unique_pid_of_process(t_shell *sh)
{
	int		fd;
	char	buffer[256];
	ssize_t	bytes_read;

	fd = open_wrap("/proc/self/stat", O_RDONLY);
	if (fd == -1)
	{
		msh_set_error(sh, OPEN_OP);
		return (-1);
	}
	bytes_read = read_wrap(fd, buffer, sizeof(buffer) - 1);
	if (bytes_read <= 0)
	{
		msh_set_error(sh, READ_OP);
		close(fd);
		return (-1);
	}
	buffer[bytes_read] = '\0';
	if (safe_close(fd) == -1)
		return (msh_set_error(NULL, CLOSE_OP), -1);
	return (ft_atoi(buffer));
}
