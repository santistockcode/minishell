/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_init.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 23:18:17 by mario             #+#    #+#             */
/*   Updated: 2026/01/11 13:09:25 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

int init_minishell(t_shell **minishell,char **envp)
{
	char ** cpy;
	*minishell = malloc(sizeof(t_shell));
	if(!minishell)
		return (MALLOC_ERROR);
	cpy =envp;
	printf("%s",cpy[1]);
	return(1);
}