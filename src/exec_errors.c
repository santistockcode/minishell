/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_errors.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 18:08:33 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/09 18:09:19 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void	msh_set_error(t_shell *sh, const char *op)
{
	char	*tmp_op;

	if (!sh)
		return ;
	free(sh->last_err_op);
	tmp_op = ft_strdup(op);
	if (!tmp_op)
		sh->last_err_op = NULL;
	else
		sh->last_err_op = tmp_op;
    sh->last_errno = errno;
}

/*
In child what to do after execve fails:
int e = errno;
int st = msh_status_from_execve_error(e);
msh_set_error_with_errno(sh, "execve", e);
msh_print_last_error(sh); _exit(st);
*/

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
	if (op && errno)
		fprintf(stderr, "minishell: %s: %s\n", op, strerror(errno));
	else if (op)
		fprintf(stderr, "minishell: %s\n", op);
	else if (errno)
		fprintf(stderr, "minishell: %s\n", strerror(errno));
	else
		fprintf(stderr, "minishell: unknown error/internal error\n");
	if (sh && sh->last_err_op)
	{
		free(sh->last_err_op);
		sh->last_err_op = NULL;
	}
}
