/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_expand.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 18:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/04 00:55:56 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/expand.h"

static char	*extract_value(char *var_name, size_t var_len, t_list *env)
{
	t_env	*var;

	var = env_get(env, var_name);
	if (!var || !var->value)
		return (NULL);
	if (ft_strlen(var->key) != var_len)
		return (NULL);
	return (ft_strdup(var->value));
}

char	*extract_varname(char *value)
{
	char	*start;
	char	*var_name;

	value++;
	var_name = NULL;
	start = value;
	if (*value == '?')
		value++;
	else
		while (ft_isalnum(*value) || *value == '_')
			value++;
	var_name = ft_substr(start, 0, value - start);
	if (!var_name)
		return (NULL);
	return (var_name);
}

char	*extract_varvalue(char *var_name, t_list *env)
{
	size_t	var_len;
	char	*var_value;

	if (!var_name || !env)
		return (ft_strdup(""));
	var_len = ft_strlen(var_name);
	var_value = extract_value(var_name, var_len, env);
	if (var_value)
		return (var_value);
	return (ft_strdup(""));
}

char	*expand_varstr(char *string, char *value, char **var_param,
		size_t *value_offset)
{
	char	*pattern;
	char	*match;
	char	*temp_array[4];
	int		i;

	i = -1;
	pattern = ft_strjoin("$", var_param[0]);
	if (!string || !pattern)
	{
		free(pattern);
		return (NULL);
	}
	match = ft_strnstr(value, pattern, ft_strlen(string));
	*value_offset = match - string;
	temp_array[0] = ft_substr(string, 0, match - string);
	temp_array[1] = ft_strdup(match + ft_strlen(pattern));
	temp_array[2] = ft_strjoin(temp_array[0], var_param[1]);
	temp_array[3] = ft_strjoin(temp_array[2], temp_array[1]);
	free(pattern);
	while (i++ < 2)
		free(temp_array[i]);
	return (temp_array[3]);
}

int	valid_varname(int c)
{
	if (c == '?')
		return (1);
	if (ft_isalnum(c) || c == '_')
		return (1);
	return (0);
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
		var_param[1] = ft_itoa(exit_status);
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
