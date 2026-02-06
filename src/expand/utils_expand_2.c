/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_expand_2.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 18:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 20:20:55 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

int	expand_and_quotes(char **string, t_list *env)
{
	int	status;
	char	*before;

	before = NULL;
	if (LOG == 1)
		before = *string ? ft_strdup(*string) : ft_strdup("(null)");
	status = expand_var_value(string, env);
	if (status != SUCCESS)
	{
		free(before);
		return (status);
	}
	status = remove_string_quotes(string);
	if (LOG == 1)
	{
		printf("[EXPAND] before=\"%s\" after=\"%s\"\n",
			before ? before : "(null)",
			*string ? *string : "(null)");
	}
	free(before);
	return (status);
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
	size_t	before_len;
	size_t	after_len;

	quote_pos = *value - *start;
	before_quote = ft_substr(*start, 0, quote_pos);
	after_quote = ft_strdup(*value + 1);
	before_len = before_quote ? ft_strlen(before_quote) : 0;
	after_len = after_quote ? ft_strlen(after_quote) : 0;
	if (before_len == 0 && after_len == 0)
		temp = ft_strdup("");
	else
		temp = ft_strjoin(before_quote, after_quote);
	if (!temp)
	{
		free(before_quote);
		free(after_quote);
		return (MALLOC_ERROR);
	}
	free(*start);
	*start = temp;
	*value = *start + quote_pos;
	free(before_quote);
	free(after_quote);
	return (SUCCESS);
}
