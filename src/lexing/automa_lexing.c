/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   automa_lexing.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 23:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 20:04:00 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

int	state_noquote(char **current, int flag)
{
	while ((msh_isprint(**current) && !ft_isspace(**current)
			&& !ft_isoperator(**current) && flag == NO_QUOTE))
	{
		flag = state_switch(*current, flag);
		if (flag != NO_QUOTE)
		{
			(*current)++;
			break ;
		}
		(*current)++;
	}
	if (ft_isspace(**current) || ft_isoperator(**current))
		flag = state_switch(*current, flag);
	return (flag);
}

int	state_singlequote(char **current, int flag)
{
	while ((msh_isprint(**current) && flag == SINGLE_QUOTE))
	{
		flag = state_switch(*current, flag);
		if (flag != SINGLE_QUOTE)
		{
			(*current)++;
			break ;
		}
		(*current)++;
	}
	return (flag);
}

int	state_doublequote(char **current, int flag)
{
	while ((msh_isprint(**current) && flag == DOUBLE_QUOTE))
	{
		flag = state_switch(*current, flag);
		if (flag != DOUBLE_QUOTE)
		{
			(*current)++;
			break ;
		}
		(*current)++;
	}
	return (flag);
}

int	state_switch(char *current, int flag)
{
	if (*current == '\'' && flag == NO_QUOTE)
		return (SINGLE_QUOTE);
	if (*current == '\'' && flag == SINGLE_QUOTE)
		return (NO_QUOTE);
	if (*current == '\"' && flag == NO_QUOTE)
		return (DOUBLE_QUOTE);
	if (*current == '\"' && flag == DOUBLE_QUOTE)
		return (NO_QUOTE);
	if ((ft_isspace(*current) || ft_isoperator(*current)) && flag == NO_QUOTE)
		return (BREAK);
	return (flag);
}
