/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 09:26:50 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/06 11:01:20 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

/*
Case scenarios (teniendo en cuenta que de un child siempre va a salir)
Note that argument can be a string "258" so we need to atoi + check out of range
	- no arguments and parent=1
		- *should_exit = 1
		- returns last_status & 0xff
	- no arguments and parent=0
		- *should_exit = 1
		- returns last_status & 0xff
	- argv0 is not numeric (even with extra args): numeric argument required
		- *should_exit = 1
		- prints minishell: exit: <argument>: numeric argument required
		- returns 2
	- argv0 valid but more than 1 argument (too many arguments)
		- *should_exit = 0
		- prints minishell: exit: too many arguments
		- returns 1
*/

void	print_exit(int parent)
{
	if (parent)
		ft_putstr_fd("exit\n", 2);
}

int	just_exit(int parent, int last_status)
{
	print_exit(parent);
	return (last_status & 0xff);
}

void	print_error_too_many(int parent)
{
	print_exit(parent);
	ft_putstr_fd("minishell: exit: too many arguments\n", 2);
}

// FIXME: is argv[1] going to be char * always?
// FIXME: undetermined behaviour in case of out of range

/*
Manages if minishell should or not exit
(if too many arguments it shouldn't).
Manages if it should print or not (if I'm in child do not print exit)
*/
int	exit_builtin(char **argv, int parent, int last_status, int *should_exit)
{
	int	status;

	if (!argv[1])
	{
		*should_exit = 1;
		return (just_exit(parent, last_status));
	}
	if (!ft_isdigit(argv[1][0]))
	{
		*should_exit = 1;
		print_exit(parent);
		ft_putstr_fd("minishell: exit: ", 2);
		ft_putstr_fd(argv[1], 2);
		ft_putstr_fd(": numeric argument required\n", 2);
		return (2);
	}
	status = ft_atoi(argv[1]);
	if (argv[2])
	{
		*should_exit = 0;
		return (print_error_too_many(parent), 1);
	}
	print_exit(parent);
	*should_exit = 1;
	return (status & 0xff);
}
