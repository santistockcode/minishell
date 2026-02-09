/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   quote_remove_expand.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 18:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/09 20:14:09 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

static int	process_quote_removal(char **value, char **start, int flag,
		int expected_flag)
{
	while ((msh_isprint(**value) && flag == expected_flag))
	{
		flag = state_switch(*value, flag);
		if (flag != expected_flag)
		{
			remove_char_quote(start, value);
			continue ;
		}
		(*value)++;
	}
	return (flag);
}

int	remove_noquote(char **value, char **start, int flag)
{
	if (!value || !*value || flag != NO_QUOTE)
	{
		return (flag);
	}
	while ((msh_isprint(**value) && !ft_isspace(**value)
			&& !ft_isoperator(**value) && flag == NO_QUOTE))
	{
		flag = state_switch(*value, flag);
		if (flag != NO_QUOTE && flag != BREAK)
		{
			remove_char_quote(start, value);
			break ;
		}
		(*value)++;
	}
	if (ft_isspace(**value) || ft_isoperator(**value))
		flag = state_switch(*value, flag);
	return (flag);
}

int	remove_quote_types(char **value, char **start, int flag)
{
	if (flag == SINGLE_QUOTE || flag == DOUBLE_QUOTE)
		return (process_quote_removal(value, start, flag, flag));
	return (flag);
}

int	remove_string_quotes(char **string)
{
	char	*start;
	char	*value;
	int		flag;
	int		loop_count;

	if (!string || !*string)
	{
		return (SUCCESS);
	}
	value = *string;
	start = value;
	flag = NO_QUOTE;
	loop_count = 0;
	while (*value && flag != BREAK)
	{
		if (flag == NO_QUOTE)
			flag = remove_noquote(&value, &start, flag);
		flag = remove_quote_types(&value, &start, flag);
		loop_count++;
		if (loop_count > 1000)
			break ;
	}
	*string = start;
	return (SUCCESS);
}
