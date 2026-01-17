/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 12:19:35 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/16 13:37:06 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"


extern volatile sig_atomic_t exit_status;

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

