/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_lexing.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/01 22:47:45 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 20:04:00 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

int	ft_isoperator(int c)
{
	char	*set;

	set = "<>|";
	if (ft_strchr(set, c))
		return (c);
	return (0);
}

int	msh_isprint(int c)
{
	unsigned char	uc;

	uc = (unsigned char)c;
	if (uc >= 32 && uc != 127)
		return (1);
	return (0);
}
