/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_init.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 23:18:17 by mario             #+#    #+#             */
/*   Updated: 2026/02/09 22:51:36 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

/* void	print_env(char **envp)
{
	int	i;

	i = 0;
	while (envp[i])
	{
		printf("\033[0;36m%s\033[0m\n", envp[i]);
		i++;
	}
}*/
int	init_minishell(t_shell **minishell, char **envp)
{
	*minishell = ft_calloc(sizeof(t_shell), 1);
	if (!(*minishell))
		return (MALLOC_ERROR);
	(*minishell)->env = init_envp(envp);
	(*minishell)->cmds_start = NULL;
	(*minishell)->exec_cmds = NULL;
	(*minishell)->lexing = NULL;
	(*minishell)->last_status = 0;
	(*minishell)->last_errno = 0;
	(*minishell)->should_exit = 0;
	(*minishell)->save_in = 0;
	(*minishell)->save_out = 0;
	(*minishell)->save_err = 0;
	(*minishell)->last_err_op = NULL;
	return (SUCCESS);
}
