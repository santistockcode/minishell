/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:37:50 by mario             #+#    #+#             */
/*   Updated: 2026/02/08 09:52:21 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

int	exec_cmds(t_shell *sh, t_list *cmd_first);
void	free_lexing(t_lexing *lexing);
void safe_free_parsing(t_list **pars_cmds);

volatile sig_atomic_t	g_exit_status = 0;

void free_env_struct(void *env)
{
    if (!env) return;
    free(((t_env*)env)->key);
    free(((t_env*)env)->value);
    free(env);
}


void free_main_loop(t_shell *minishell)
{
	if(minishell->lexing)
		free_lexing(minishell->lexing);
	if(minishell->pars_cmds)
		free_commands(minishell->pars_cmds);
	if(minishell->exec_cmds)
		free_cmds(minishell->exec_cmds);
	minishell->lexing = NULL;
	minishell->pars_cmds = NULL;
	minishell->exec_cmds = NULL;
}
void free_shell(t_shell *sh)
{
	//free_main_loop(sh);
	if (!sh) 
		return;
	ft_lstclear(&sh->env, free_env_struct);
	if (sh->last_err_op)
		free(sh->last_err_op);
	free(sh);
}

/*
MANUAL TESTING (HAPPY PATH):

- En caso de que le usuarie introduzca 'echo "hello"', compruebo en el logger de la parte de ejecución que args[1] es hello y no "hello" como sucede ahora.

- Después de entrar 0 veces en el bucle no tenemos leaks

- Después de entrar 1 vez en el bucle, minishell debe quedar tal que: 
	t_list		*env; // allocated
	t_lexing	*lexing; // liberado (ya lo hemmos probado)
	t_list		*cmds_start; // NULL (tanto antes de llamar a exec_cmds como a la salida debe ser NULL)
	t_list		*pars_cmds; // liberado (vamos por aquí)
	t_list		*exec_cmds;	// liberado
	int			last_status; // set last $? value
	int			should_exit; // debe seguir siendo 0 si no se ha llamado a exit
	char		*last_err_op; // seguir siendo null dado que no ha habido un error
	int			last_errno;	// 0 dado que no ha habido un error
	int			save_in; // 0
	int			save_out; // 0
	int			save_err; // 0

- Después de entrar en el bucle, hacer control+C y salir, minishell debe quedar tal que:

- Después de entrar en el bucle, hacer control+D y salir, minishell debe quedar tal que:

- Después de interrumpir el tratado de here_docs y volver al bucle, minishell debe quedar tal que:

- Todos los casos anteriores pero con --trace-child en valgrind. 

- segfault cuando unset es null

- echo $UNEXISTANT-VARIABLE se queda colgado

- Comandos con argumento entrecomillado, este argumento no llega a la parte de ejecución.

- hay un salto de línea raro en cada bucle

- exit no sale con el código que debekill -9

MANUAL TESTING (UNHAPPY PATH):
(pendiente)

*/



int main(int argc, char** argv, char **envp)
{
	t_shell	*minishell;
	int status;
	
	//int bucle = 0;
	status = 0;
	if(argc != 0 && argv[1] != NULL)
		return(0);
	if (init_minishell(&minishell, envp))
		return (MALLOC_ERROR);
	setup_signal();
	while (minishell->should_exit == 0)
	{
		if (lexing(minishell) == EOF)
			break;
		if (not_tokens(minishell) != 0)
			continue;
		add_history(minishell->lexing->buff);
		parsing(minishell);
		expand_variables(minishell);
		set_to_exec(minishell);
		status = exec_cmds(minishell, minishell->exec_cmds);
		free_main_loop(minishell);
		// TODO: set important issues of minishell structure back to 0
		minishell->last_status = status;
	}
	free_shell(minishell);
	return (status);
}
