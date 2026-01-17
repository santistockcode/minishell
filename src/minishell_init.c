/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_init.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 23:18:17 by mario             #+#    #+#             */
/*   Updated: 2026/01/16 12:30:17 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

/* void	print_env(char **envp)
{
	int	i = 0;

	while (envp[i])
	{
		printf("\033[0;36m%s\033[0m\n", envp[i]);
		i++;
	}
}*/

int init_minishell(t_shell **minishell,char **envp)
{
	*minishell = ft_calloc(sizeof(t_shell),1);
	if(!minishell)
		return (MALLOC_ERROR);
	(*minishell)->env = init_envp(envp);
	//print_env(envp);
	print_env_list((*minishell)->env);
	return(1);
}