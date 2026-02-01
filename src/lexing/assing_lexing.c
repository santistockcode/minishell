/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   assing_lexing_list.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 23:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/01 18:10:40 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"


void	assign_redir_token(char **current, t_token **new_token)
{
	if (ft_strncmp(*current, "<<", 2) == 0)
	{
		(*new_token)->value = ft_substr(*current, 0, 2);
		(*new_token)->type = TOKEN_HEREDOC;
		*current += 2;
	}
	else if (ft_strncmp(*current, ">>", 2) == 0)
	{
		(*new_token)->value = ft_substr(*current, 0, 2);
		(*new_token)->type = TOKEN_REDIR_APPEND;
		*current += 2;
	}
	else if (ft_strncmp(*current, "<", 1) == 0)
	{
		(*new_token)->value = ft_substr(*current, 0, 1);
		(*new_token)->type = TOKEN_REDIR_IN;
		(*current)++;
	}
	else if (ft_strncmp(*current, ">", 1) == 0)
	{
		(*new_token)->value = ft_substr(*current, 0, 1);
		(*new_token)->type = TOKEN_REDIR_OUT;
		(*current)++;
	}
}

void	assign_pipevar_token(char **current, t_token **new_token)
{
	if (ft_strncmp(*current, "|", 1) == 0)
	{
		(*new_token)->value = ft_substr(*current, 0, 1);
		(*new_token)->type = TOKEN_PIPE;
		(*current)++;
	}
}

void	assign_var_token(t_token *token)
{
	char	*equal_pos;
	char	*var_name;
	int		i;
	int		flag;

	flag = 0;
	i = 0;
	equal_pos = ft_strchr(token->value, '=');
	if (!equal_pos)
		return ;
	var_name = ft_substr(token->value, 0, equal_pos - token->value);
	if (!var_name)
		return ;
	if (ft_strchr(var_name, '\'') || ft_strchr(var_name, '\"'))
		flag = 1;
	while (var_name[i])
	{
		if (!ft_isalnum(var_name[i]) && var_name[i] != '_')
			flag = 1;
		i++;
	}
	if (flag == 0)
		token->type = TOKEN_ASSIGN_WORD;
	free(var_name);
}

void	assign_word_token(char **current, t_token **new_token)
{
	char	*start;
	int		flag;

	start = *current;
	flag = NO_QUOTE;
	while (**current && flag != BREAK)
	{
		if (flag == NO_QUOTE)
			flag = state_noquote(current, flag);
		else if (flag == SINGLE_QUOTE)
			flag = state_singlequote(current, flag);
		else if (flag == DOUBLE_QUOTE)
			flag = state_doublequote(current, flag);
		else
			(*current)++;
	}
	(*new_token)->value = ft_substr(start, 0, *current - start);
	if (flag != NO_QUOTE && flag != BREAK)
	{
		if (flag == SINGLE_QUOTE)
			(*new_token)->syntax_error = SYNTAX_SINGLE_QUOTE;
		if (flag == DOUBLE_QUOTE)
			(*new_token)->syntax_error = SYNTAX_DOUBLE_QUOTE;
	}
	(*new_token)->type = TOKEN_WORD;
}
