/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   quote_remove_expand.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 18:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 20:04:00 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

int	remove_string_quotes(char **string)
{
	char	*start;
	char	*value;
	int		flag;

	if (!string || !*string)
		return (SUCCESS);
	value = *string;
	start = value;
	flag = NO_QUOTE;
	while (*value && flag != BREAK)
	{
		if (flag == NO_QUOTE)
			flag = remove_noquote(&value, &start, flag);
		if (flag == DOUBLE_QUOTE)
			flag = remove_doublequote(&value, &start, flag);
		if (flag == SINGLE_QUOTE)
			flag = remove_singlequote(&value, &start, flag);
	}
	*string = start;
	return (SUCCESS);
}

int	remove_noquote(char **value, char **start, int flag)
{
	if (!value || !*value || flag != NO_QUOTE)
		return (flag);
	while ((msh_isprint(**value) && !ft_isspace(**value)
			&& !ft_isoperator(**value) && flag == NO_QUOTE))
	{
		flag = state_switch(*value, flag);
		if (flag != NO_QUOTE && flag != BREAK)
		{
			remove_char_quote(start, value);
			continue ;
		}
		(*value)++;
	}
	if (ft_isspace(**value) || ft_isoperator(**value))
		flag = state_switch(*value, flag);
	return (flag);
}

int	remove_singlequote(char **value, char **start, int flag)
{
	if (!value || !*value || flag != SINGLE_QUOTE)
		return (flag);
	while ((msh_isprint(**value) && flag == SINGLE_QUOTE))
	{
		flag = state_switch(*value, flag);
		if (flag != SINGLE_QUOTE)
		{
			remove_char_quote(start, value);
			continue ;
		}
		(*value)++;
	}
	return (flag);
}

int	remove_doublequote(char **value, char **start, int flag)
{
	if (!value || !*value || flag != DOUBLE_QUOTE)
		return (flag);
	while ((msh_isprint(**value) && flag == DOUBLE_QUOTE))
	{
		flag = state_switch(*value, flag);
		if (flag != DOUBLE_QUOTE)
		{
			remove_char_quote(start, value);
			continue ;
		}
		(*value)++;
	}
	return (flag);
}
