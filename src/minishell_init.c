/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_init.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 23:18:17 by mario             #+#    #+#             */
/*   Updated: 2026/01/11 19:46:05 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

int init_minishell(t_shell **minishell,char **envp)
{
	char ** cpy;
	*minishell = ft_calloc(sizeof(t_shell),1);
	if(!minishell)
		return (MALLOC_ERROR);
	cpy = envp;
	// printf("%s",cpy[1]);
	return(1);
}