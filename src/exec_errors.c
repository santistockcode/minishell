/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_errors.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 18:08:33 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/31 17:59:24 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

int	msh_status_from_execve_error(int err)
{
	if (err == ENOENT)
		return (STATUS_CMD_NOT_FOUND);
	if (err == EACCES || err == EPERM || err == EISDIR || err == ENOEXEC)
		return (STATUS_CMD_NOT_EXEC);
	return (1);
}

void	msh_set_error(t_shell *sh, const char *op)
{
	char	*tmp_op;

	if (!sh)
		return ;
	sh->last_errno = errno;
	free(sh->last_err_op);
	tmp_op = ft_strdup(op);
	if (!tmp_op)
		sh->last_err_op = NULL;
	else
		sh->last_err_op = tmp_op;
}

void	putstr_fd_err(int n, ...)
{
	va_list	args;
	int		i;

	va_start(args, n);
	i = 0;
	while (i < n)
	{
		ft_putstr_fd(va_arg(args, char *), 2);
		i++;
	}
	ft_putstr_fd("\n", 2);
	va_end(args);
}

/*
When a syscall fails, it often sets errno. Except that we are still
using ft_strdup above in msh_set_error, which may as well fail. 
That's the reason to have a fallback "unknown error/internal error" message.
*/
void	msh_print_last_error(t_shell *sh)
{
	char	*op;

	op = NULL;
	if (sh && sh->last_err_op)
		op = sh->last_err_op;
	if (op && sh && sh->last_errno)
		putstr_fd_err(4, "minishell: ", op, ": ", strerror(sh->last_errno));
	else if (op)
		putstr_fd_err(2, "minishell: ", op);
	else if (sh && errno)
		putstr_fd_err(2, "minishell: ", strerror(errno));
	else
		putstr_fd_err(1, "minishell: unknown error/internal error");
	if (sh && sh->last_err_op)
	{
		free(sh->last_err_op);
		sh->last_err_op = NULL;
	}
	sh->last_errno = 0;
}

/*
In child what to do after execve fails:
int e = errno;
int st = msh_status_from_execve_error(e);
msh_set_error_with_errno(sh, "execve", e);
msh_print_last_error(sh); _exit(st);
*/
