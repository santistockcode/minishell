/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_hd.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 15:11:45 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/13 16:29:02 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include "../include/exec.h"
#include "../include/log.h"

int					get_env_from_key(char *key, t_list *env, char **res_value);
int					add_char_to_list(char c, t_list **chars_list);
int					add_string_to_list(char *str, t_list **chars_list);
void				free_aux_list(t_list **lst);
char				*list_to_alloc_string(t_list *chars_list);

/*
Expands a variable if exists in env and adds it to the chars_list.
*/
int	expand_variable_from(char *line, t_list **chars_list, t_list *env)
{
	int		var_size;
	char	*var_value;
	char	*tmp_no_dollar;
	int		var_result;

	var_size = 0;
	while (line[var_size] && !ft_isspace(line[var_size])
		&& line[var_size] != '$')
		var_size++;
	tmp_no_dollar = ft_substr(line, 0, var_size);
	if (!tmp_no_dollar)
		return (0);
	var_result = get_env_from_key(tmp_no_dollar, env, &var_value);
	if (var_result && var_result < 0)
		return (free(tmp_no_dollar), 0);
	if (!var_value)
		return (1);
	else
	{
		if (add_string_to_list(var_value, chars_list) == 0)
			return (free(var_value), 0);
	}
	free(var_value);
	return (1);
}

/*
Expands a variable and moves index to end of the word.
*/
int	expand_variable_dollar_case(const char *content, int i,
	t_list **chars_list, t_list *env)
{
	if (expand_variable_from((char *)(content + i + 1),
		chars_list, env) == 0)
		return (-1);
	i++;
	while (!ft_isspace(content[i]) && content[i] != '$'
		&& content[i] != '\0')
		i++;
	return (i);
}

/*
Manages dollar case expansion, both variable from env and $?
*/
int	special_dollar_cases(const char *content, int i, t_list **chars_list,
		t_shell *sh)
{
	char	*tmp_last_status;

	if (ft_isspace(content[i + 1]) || content[i + 1] == '\0'
		|| content[i + 1] == '$')
	{
		if (add_char_to_list(content[i], chars_list) == 0)
			return (-1);
		i++;
	}
	else if (content[i + 1] == '?')
	{
		tmp_last_status = ft_itoa(sh->last_status);
		if (!tmp_last_status)
			return (-1);
		if (add_string_to_list(tmp_last_status, chars_list) == 0)
			return (free(tmp_last_status), -1);
		i += 2;
		free(tmp_last_status);
	}
	return (i);
}

/*
Expands fetched line from user (content), $VAR, $? and $ as literal
Returns an empty list on content empty "". 
Returns NULL on malloc error (no other).
*/
t_list	*expand_heredoc_content_to_list(const char *content, t_shell *sh)
{
	int		i;
	t_list	*chars_list;

	i = 0;
	chars_list = NULL;
	while (content && content[i] != '\0')
	{
		if (content[i] == '$' && (ft_isspace(content[i + 1])
				|| content[i + 1] == '\0' || content[i + 1] == '$'
				|| content[i + 1] == '?'))
			i = special_dollar_cases(content, i, &chars_list, sh);
		else if (content[i] == '$')
			i = expand_variable_dollar_case(content, i, &chars_list, sh->env);
		else
		{
			if (add_char_to_list(content[i], &chars_list) == 0)
				return (free_aux_list(&chars_list), NULL);
			i++;
		}
		if (i == -1)
			return (free_aux_list(&chars_list), NULL);
	}
	if (!chars_list)
		return (ft_lstnew(NULL));
	return (chars_list);
}

/*
Expands heredoc content by using a chars list;
Returns dynamically allocated string.
Returns NULL on error (and is always a malloc error)
*/
char	*expand_hd(const char *content, t_shell *sh)
{
	t_list	*chars_list;
	char	*final_line;

	if (!content || *content == '\0')
	{
		logger("expand_hd", "content is empty");
		chars_list = ft_lstnew(NULL);
		if (!chars_list)
			return (NULL);
	}
	else
	{
		chars_list = expand_heredoc_content_to_list(content, sh);
		if (!chars_list)
			return (NULL);
	}
	final_line = list_to_alloc_string(chars_list);
	if (!final_line)
		return (free_aux_list(&chars_list), NULL);
	free_aux_list(&chars_list);
	return (final_line);
}
