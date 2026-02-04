/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   quote_state_expand.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 18:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/04 18:09:01 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

int	expand_noquote(char **value, char **start, int flag, t_list *env)
{
	while ((ft_isprint(**value) && !ft_isspace(**value)
			&& !ft_isoperator(**value) && flag == NO_QUOTE))
	{
		flag = state_switch(*value, flag);
		if (flag != NO_QUOTE)
		{
			(*value)++;
			break ;
		}
		else if (**value == '$')
		{
			if (ft_strncmp("$$", *value, 2) == 0)
				(*value) += 1;
			else if (*(*value + 1) == '\0' || !valid_varname(*(*value + 1)))
				(*value)++;
			else
			{
				expand_and_replace(value, start, env);
				flag = state_switch(*value, flag);
			}
			continue ;
		}
		(*value)++;
	}
	return (flag);
}

int	expand_singlequote(char **value, int flag)
{
	while ((ft_isprint(**value) && flag == SINGLE_QUOTE))
	{
		flag = state_switch(*value, flag);
		if (flag != SINGLE_QUOTE)
		{
			(*value)++;
			break ;
		}
		(*value)++;
	}
	return (flag);
}

int	expand_doublequote(char **value, char **start, int flag, t_list *env)
{
	while ((ft_isprint(**value) && flag == DOUBLE_QUOTE))
	{
		flag = state_switch(*value, flag);
		if (flag != DOUBLE_QUOTE)
		{
			(*value)++;
			break ;
		}
		else if (**value == '$')
		{
			if (ft_strncmp("$$", *value, 2) == 0)
				(*value) += 1;
			else if (*(*value + 1) == '\0' || !valid_varname(*(*value + 1)))
				(*value)++;
			else
			{
				expand_and_replace(value, start, env);
				flag = state_switch(*value, flag);
			}
			continue ;
		}
		(*value)++;
	}
	return (flag);
}

int	expand_var_value(char **string, t_list *env)
{
	char	*start;
	char	*value;
	int		flag;

	if (!string)
		return (INPUT_ERROR);
	if (!*string)
		*string = ft_strdup("");
	value = *string;
	start = value;
	flag = NO_QUOTE;
	while (*value && flag != BREAK)
	{
		if (flag == NO_QUOTE)
			flag = expand_noquote(&value, &start, flag, env);
		else if (flag == SINGLE_QUOTE)
			flag = expand_singlequote(&value, flag);
		else if (flag == DOUBLE_QUOTE)
			flag = expand_doublequote(&value, &start, flag, env);
		if (ft_isspace(*value) || ft_isoperator(*value))
			flag = state_switch(value, flag);
	}
	*string = start;
	return (SUCCESS);
}
