#include "../include/minishell.h"
#include "../include/exec.h"

int	expand_variable_from(char *line, t_list **chars_list, t_list *env)
{
	int		var_size;
	char	*var_value;
	char	*tmp_no_dollar;

	var_size = 0;
	while (line[var_size] && !ft_isspace(line[var_size])
		&& line[var_size] != '$')
		var_size++;
	tmp_no_dollar = ft_substr(line, 0, var_size);
	if (!tmp_no_dollar)
		return (0);
	var_value = get_env_from_key(tmp_no_dollar, env);
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
			return (-1);
		i += 2;
		free(tmp_last_status);
	}
	return (i);
}

/*
*/
t_list	*expand_heredoc_content_to_list(const char *content, t_shell *sh)
{
	int		i;
	char	current_char;
	t_list	*chars_list;

	i = 0;
	chars_list = NULL;
	while (content[i] != '\0')
	{
		current_char = content[i];
		if (current_char == '$' && (ft_isspace(content[i + 1])
				|| content[i + 1] == '\0' || content[i + 1] == '$'
				|| content[i + 1] == '?'))
			i = special_dollar_cases(content, i, &chars_list, sh);
		else if (current_char == '$')
			i = expand_variable_dollar_case(content, i, &chars_list, sh->env);
		else
		{
			if (add_char_to_list(current_char, &chars_list) == 0)
				return (free_aux_list(&chars_list), NULL);
			i++;
		}
		if (i == -1)
			return (free_aux_list(&chars_list), NULL);
	}
	return (chars_list);
}

/*
FIXME: expand_heredoc_content_to_list is both scanning and expanding, 
though syscall failure is not managable (empty list is ugly as fuck)
*/
char	*expand_hd(const char *content, t_shell *sh)
{
	t_list	*chars_list;
	char	*final_line;

	if (!content || *content == '\0')
	{
		chars_list = ft_lstnew(NULL);
		if (!chars_list)
			return (NULL);
	}
	else
		chars_list = expand_heredoc_content_to_list(content, sh);
	if ((content && *content != '\0') && !chars_list)
		chars_list = ft_lstnew(NULL);
	if (!chars_list)
		return (NULL);
	final_line = list_to_alloc_string(chars_list);
	if (!final_line)
		return (free_aux_list(&chars_list), NULL);
	free_aux_list(&chars_list);
	return (final_line);
}
