/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_expand_2.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 18:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/08 11:12:14 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

int	expand_and_quotes(char **string, t_list *env)
{
	int	status;

	status = expand_var_value(string, env);
	if (status != SUCCESS)
		return (status);
	status = remove_string_quotes(string);
	return (status);
}

static char	*get_var_param_value(char *varname, t_list *env)
{
	if (ft_strncmp("?", varname, 1) == 0)
		return (ft_itoa(g_exit_status));
	return (extract_varvalue(varname, env));
}

static int	update_start_with_value(char **value, char **start,
		char **var_param, size_t *value_offset)
{
	char	*new_value;

	new_value = expand_varstr(*start, *value, var_param, value_offset);
	if (!new_value)
		return (MALLOC_ERROR);
	free(*start);
	*start = new_value;
	*value = *start + *value_offset;
	return (SUCCESS);
}

int	expand_and_replace(char **value, char **start, t_list *env)
{
	char	*var_param[2];
	size_t	value_offset;
	int		status;

	var_param[0] = extract_varname(*value);
	if (!var_param[0])
		return (MALLOC_ERROR);
	var_param[1] = get_var_param_value(var_param[0], env);
	status = update_start_with_value(value, start, var_param, &value_offset);
	free(var_param[0]);
	free(var_param[1]);
	return (status);
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
	if (!temp)
		return (free(before_quote), free(after_quote), MALLOC_ERROR);
	free(*start);
	*start = temp;
	*value = *start + quote_pos;
	free(before_quote);
	free(after_quote);
	return (SUCCESS);
}
