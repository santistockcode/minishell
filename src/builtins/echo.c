/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   echo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 17:45:54 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/06 09:27:45 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

/*
 * Counts the number of -n flags in the argv array.
 */
int	count_n_flags(char **argv)
{
	int	count;
	int	i;
	int	j;

	count = 0;
	i = 1;
	while (argv[i])
	{
		if (argv[i][0] != '-' || argv[i][1] != 'n')
			break ;
		j = 1;
		while (argv[i][j] == 'n')
			j++;
		if (argv[i][j] != '\0')
			break ;
		count++;
		i++;
	}
	return (count);
}

/*
argv[0] = 'echo'
Don't start printing until n flags end
*/
void	echo_builtin(char **argv)
{
	char	*to_print;
	int		flag;
	int		i;

	flag = 0;
	i = 1;
	if (argv[i] && ft_strncmp(argv[i], "-n", 2) == 0)
		flag = 1;
	i = count_n_flags(argv++);
	while (argv[i])
	{
		to_print = argv[i];
		if (to_print)
		{
			ft_putstr_fd(to_print, STDOUT_FILENO);
			i++;
			if (argv[i])
				ft_putchar_fd(' ', STDOUT_FILENO);
		}
		else
			break ;
	}
	if (!flag)
		ft_putchar_fd('\n', STDOUT_FILENO);
}
