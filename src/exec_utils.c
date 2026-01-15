/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 12:19:35 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/15 16:44:33 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Libft/include/libft.h"
#include "../include/exec.h"
#include "../include/syswrap.h"
#include "../include/log.h"
#include <stdio.h>
#include <stdlib.h>

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

/*
FIXME: evolve this function once pipex integrated
*/
int	calculate_status_from_errno(int exit_status)
{
	int	es_result;

	es_result = exit_status + 128;
	return (es_result);
}
