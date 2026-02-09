/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_expand.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 18:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/09 16:14:28 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

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
	char	*match;
	char	*before;
	char	*after;
	char	*result;
	size_t	pattern_len;

	match = find_var_match(string, value, var_param[0], value_offset);
	if (!match)
		return (NULL);
	pattern_len = ft_strlen(var_param[0]) + 1;
	before = ft_substr(string, 0, match - string);
	after = ft_strdup(match + pattern_len);
	result = join_parts(before, var_param[1], after);
	free(before);
	free(after);
	return (result);
}

int	valid_varname(int c)
{
	if (c == '?')
		return (1);
	if (ft_isalnum(c) || c == '_')
		return (1);
	return (0);
}
