/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:37:50 by mario             #+#    #+#             */
/*   Updated: 2026/02/07 11:09:10 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

int	exec_cmds(t_shell *sh, t_list *cmd_first);

volatile sig_atomic_t	g_exit_status = 0;

int main(int argc, char** argv, char **envp)
{
	t_shell	*minishell;
	t_list	*commands;

	if(argc != 0 && argv[1] != NULL)
		return(0);
	if (init_minishell(&minishell, envp))
		return (MALLOC_ERROR);
	setup_signal();
	while (1 && minishell->should_exit == 0)
	{
		logger("[main]", "line 32");
		if (lexing(minishell) == EOF)
			break;
		if (not_tokens(minishell) != 0)
			continue;
		add_history(minishell->lexing->buff);
		parsing(minishell);
		expand_variables(minishell);
		set_to_exec(minishell);
		exec_cmds(minishell, commands);
		//free(lexing parsing and ejecution)
	}
	free(minishell);
	return (0);
}
