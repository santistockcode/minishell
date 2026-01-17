/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_init.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 23:18:17 by mario             #+#    #+#             */
/*   Updated: 2026/01/18 00:17:09 by mnieto-m         ###   ########.fr       */
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
	(*minishell)->term_token = ft_calloc(sizeof(t_term_token), 1);
	if (!(*minishell)->term_token)
	(*minishell)->env = init_envp(envp);
	(*minishell)->local_var = malloc(sizeof(t_vector));
	if (!(*minishell)->local_var)
		return (MALLOC_ERROR);
	
	return(SUCCESS);
}