/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:37:50 by mario             #+#    #+#             */
/*   Updated: 2026/02/02 18:47:55 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

volatile sig_atomic_t exit_status = 0;

int main(int argc, char** argv, char **envp)
{
	t_shell	*minishell;

	if(argc != 0 && argv[1] != NULL)
		return(0);
	if (init_minishell(&minishell, envp))
		return (MALLOC_ERROR);
	
	setup_signal();
	while (1)
	{
		if (lexing(minishell) == EOF)
			break;
		if (not_tokens(minishell) != 0)
			continue;
		add_history(minishell->lexing->buff);
		// parsing 
		// ft_set_to_exec()
		//set_here_docs(sh, cmds)
		// exec_cmds (sh, cmds)
		//free(lexing parsing and ejecution)
	}
	
	logger("main", "t_shell structure initialized by Mario");
	free(minishell);
	return (0);
}
