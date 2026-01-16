/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:37:50 by mario             #+#    #+#             */
/*   Updated: 2026/01/13 17:39:48 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../include/log.h"

int main(int argc, char** argv,char **envp)
{
	t_shell	*minishell;

	if(argc != 0 && argv[1] != NULL)
		return(0);
	if (!init_minishell(&minishell, envp))
		return (MALLOC_ERROR);
	setup_signal();
	// while(1)
	// {
		//redline
		//add_history
		// parsing 
		// lexing
		// ft_set_to_exec()
			//set_here_docs(sh, cmds)
		// exec_cmds (sh, cmds)
	// }
	logger("main", "t_shell structure initialized by Mario");
	free(minishell);// mega free (free_sh and free_cmds)
	return (0);
}
