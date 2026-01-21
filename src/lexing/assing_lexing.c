/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   assing_lexing.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 22:49:38 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/01/20 22:49:52 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void	assign_redir_token(char **term_i, t_token **new_token)
{
	if (ft_strncmp(*term_i, "<<", 2) == SUCCESS)
	{
		(*new_token)->value = ft_substr(*term_i, 0, 2);
		(*new_token)->type = TOKEN_HEREDOC;
		*term_i += 2;
	}
	else if (ft_strncmp(*term_i, ">>", 2) == SUCCESS)
	{
		(*new_token)->value = ft_substr(*term_i, 0, 2);
		(*new_token)->type = TOKEN_REDIR_APPEND;
		*term_i += 2;
	}
	else if (ft_strncmp(*term_i, "<", 1) == SUCCESS)
	{
		(*new_token)->value = ft_substr(*term_i, 0, 1);
		(*new_token)->type = TOKEN_REDIR_IN;
		(*term_i)++;
	}
	else if (ft_strncmp(*term_i, ">", 1) == SUCCESS)
	{
		(*new_token)->value = ft_substr(*term_i, 0, 1);
		(*new_token)->type = TOKEN_REDIR_OUT;
		(*term_i)++;
	}
}

void	assign_pipevar_token(char **term_i, t_token **new_token)
{
	if (ft_strncmp(*term_i, "|", 1) == SUCCESS)
	{
		(*new_token)->value = ft_substr(*term_i, 0, 1);
		(*new_token)->type = TOKEN_PIPE;
		(*term_i)++;
	}
}

void	assign_var_token(t_token *token)
{
	char	*equal_pos;
	char	*var_name;
	int		i;
	int		flag;

	flag = 0;
	equal_pos = ft_strchr(token->value, '=');
	if (!equal_pos)
		return ;
	var_name = ft_substr(token->value, 0, equal_pos - token->value);
	if (!var_name)
		return ;
	if (ft_strchr(var_name, '\'') || ft_strchr(var_name, '\"'))
		flag = 1;
	i = 0;
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