/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_expand_2.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 18:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 16:50:13 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

int	expand_and_quotes(char **string, t_list *env)
{
	int	status;

	status = expand_var_value(string, env);
	if (status != SUCCESS)
		return (status);
	return (remove_string_quotes(string));
}

int	expand_and_replace(char **value, char **start, t_list *env)
{
	char	*var_param[2];
	char	*new_value;
	size_t	value_offset;

	var_param[0] = extract_varname(*value);
	if (!var_param[0])
		return (MALLOC_ERROR);
	if (ft_strncmp("?", var_param[0], 1) == 0)
		var_param[1] = ft_itoa(g_exit_status);
	else
		var_param[1] = extract_varvalue(var_param[0], env);
	new_value = expand_varstr(*start, *value, var_param, &value_offset);
	if (!new_value)
	{
		free(var_param[0]);
		free(var_param[1]);
		return (MALLOC_ERROR);
	}
	free(*start);
	*start = new_value;
	*value = *start + value_offset;
	free(var_param[0]);
	free(var_param[1]);
	return (SUCCESS);
}

int	remove_char_quote(char **start, char **value)
{
	char	*before_quote;
	char	*after_quote;
	char	*temp;
	size_t	quote_pos;

	quote_pos = *value - *start;
	before_quote = ft_substr(*start, 0, quote_pos);
	after_quote = ft_strdup(*value + 1);
	temp = ft_strjoin(before_quote, after_quote);
	free(*start);
	*start = temp;
	*value = *start + quote_pos;
	free(before_quote);
	free(after_quote);
	return (SUCCESS);
}
