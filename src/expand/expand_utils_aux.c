/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_utils_aux.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 16:10:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/09 16:14:26 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

char	*find_var_match(char *string, char *value, char *var_name,
		size_t *offset)
{
	char	*pattern;
	char	*match;

	pattern = ft_strjoin("$", var_name);
	if (!string || !pattern)
		return (free(pattern), NULL);
	match = ft_strnstr(value, pattern, ft_strlen(string));
	free(pattern);
	if (!match)
		return (NULL);
	*offset = match - string;
	return (match);
}

char	*join_parts(char *before, char *var_val, char *after)
{
	char	*temp;
	char	*result;
	int		before_empty;
	int		value_empty;
	int		after_empty;

	before_empty = (before && before[0] == '\0');
	value_empty = (var_val && var_val[0] == '\0');
	temp = ft_strjoin(before, var_val);
	if (!temp && before_empty && value_empty)
		temp = ft_strdup("");
	if (!temp)
		return (NULL);
	after_empty = (after && after[0] == '\0');
	result = ft_strjoin(temp, after);
	if (!result && temp[0] == '\0' && after_empty)
		result = ft_strdup("");
	free(temp);
	return (result);
}
