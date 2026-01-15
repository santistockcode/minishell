/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:37:50 by mario             #+#    #+#             */
/*   Updated: 2026/01/15 15:32:38 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

volatile sig_atomic_t exit_status = 0;

int main(int argc, char** argv,char **envp)
{
	t_shell *minishell;

	if(argc != 0 && argv[1] != NULL)
		return(0);
	if (!init_minishell(&minishell, envp))
		return (MALLOC_ERROR);
	minishell->i = 10;
	MSH_LOG("Minishell initialized with i = %d", minishell->i);
	setup_signal();
	while(1)
	{

	}
	free(minishell);
	return (0);
}
