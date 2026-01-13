/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   set_here_docs.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 15:38:09 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/13 16:27:13 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Libft/include/libft.h"
#include "../include/exec.h"
#include "../include/syswrap.h"
#include "../include/log.h"
#include <stdio.h>
#include <stdlib.h>

/*
Note on syscalls: 
Entry point set_here_docs will return (-1) on any syscall fail.
Possible syscalls errors up to this point include: 
- readline
- open
- malloc
Except malloc, all have been tested to not have leaks under failure. 
All malloc calls and calls to libft that makes use of malloc are protected
*/

/*
This function processes the suffix with the current shell's unique process ID.
*/
char	*process_suffix_with_pid(int suffix, t_shell *sh)
{
	int		pid;
	char	*sfx_cmd;
	char	*pid_sfx;
	char	*result;

	pid = get_unique_pid_of_process(sh);
	if (pid == -1)
		return (NULL);
	sfx_cmd = ft_itoa(suffix);
	if (!sfx_cmd)
		return (msh_set_error(sh, MALLOC_OP), NULL);
	pid_sfx = ft_itoa(pid);
	if (!pid_sfx)
		return (free(sfx_cmd), msh_set_error(sh, MALLOC_OP), NULL);
	result = ft_strjoin(sfx_cmd, pid_sfx);
	if (!result)
		return (msh_set_error(sh, MALLOC_OP),
			free(sfx_cmd), free(pid_sfx), NULL);
	free(sfx_cmd);
	free(pid_sfx);
	return (result);
}

/*
Uses readline to fetch user input for the heredoc, and expands it
with expand_hd. Then writes expanded line on fd.
Returns -1 and frees line on error
*/
int	repl_here_doc(t_shell *sh, const char *delim, int should_expand, int fd)
{
	char	*line;
	char	*expanded_line;

	while (1 && exit_status != 130)
	{
		line = readline_wrap("> ");
		if (!line)
			return (msh_set_error(sh, READLINE_OP), -1);
		if (ft_strncmp(line, delim, ft_strlen(delim)) == 0)
		{
			free(line);
			break ;
		}
		if (should_expand == 1)
		{
			expanded_line = expand_hd((const char *) line, sh);
			if (!expanded_line)
				return (free(line), msh_set_error(sh, MALLOC_OP), -1);
			free(line);
			line = expanded_line;
		}
		ft_putendl_fd(line, fd);
		free(line);
	}
	return (0);
}

/*
Opens a file descriptor for the heredoc with unique name and
calls repl_here_doc. On error returns -1. Else returns 0.
*/
int	fetch_hd_from_user(t_shell *sh, char **delim,
	int should_expand, int suffix)
{
	int		fd;
	char	*here_doc_name;
	char	*unique_sfx;
	int		rpl_result;

	unique_sfx = process_suffix_with_pid(suffix, sh);
	if (!unique_sfx)
		return (-1);
	here_doc_name = ft_strjoin(".here_doc_", unique_sfx);
	if (!here_doc_name)
		return (free(unique_sfx), msh_set_error(sh, MALLOC_OP), -1);
	free(unique_sfx);
	fd = open_wrap(here_doc_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
	{
		free(here_doc_name);
		return (msh_set_error(sh, OPEN_OP), -1);
	}
	rpl_result = repl_here_doc(sh, *delim, should_expand, fd);
	free(*delim);
	*delim = here_doc_name;
	if (rpl_result == -1)
		return (safe_close(fd), -1);
	safe_close(fd);
	return (0);
}

/*
Processes a heredoc by fetching its content from the user.
Logs error in case of error.
*/
int	process_here_doc(t_shell *sh, t_redir *redir, int suffix)
{
	int	result;

	setup_signals_heredoc();
	result = fetch_hd_from_user(sh, &(redir->target),
			!(redir->quoted), suffix);
	if (result != 0)
	{
		logger("process_here_doc", "Failed to fetch heredoc content for cmd number");
		return (result);
	}
	return (0);
}

/*
This functions iterates over commands and if finds one with redir 
R_HEREDOC, it will fetch the heredoc content from the user..
Returns -1 on any fail, 0 on success.
*/
int	set_here_docs(t_shell *sh, t_list *cmd_first)
{
	t_list	*cmd_list;
	t_list	*redir_list;
	t_redir	*redir;
	int		suffix;

	cmd_list = cmd_first;
	suffix = 0;
	logger_ctx(sh, cmd_first, "set_here_docs", "Starting to process here docs");
	while (cmd_list)
	{
		redir_list = ((t_cmd *)(cmd_list->content))->redirs;
		while (redir_list)
		{
			redir = (t_redir *)redir_list->content;
			if (redir && redir->type == R_HEREDOC)
			{
				if (process_here_doc(sh, redir, suffix++) != 0)
					return (-1);
				break ;
			}
			redir_list = redir_list->next;
		}
		cmd_list = cmd_list->next;
	}
	return (0);
}
