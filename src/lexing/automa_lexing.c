/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   automa_lexing.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 20:40:30 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/01/18 20:42:26 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void	assign_word_token(char **term_i, t_token **new_token) // remove expand
{
	char	*start;
	int		flag;

	start = *term_i;
	flag = NO_QUOTE;
	while (**term_i && flag != BREAK)
	{
		if (flag == NO_QUOTE)
			flag = state_noquote(term_i, flag);
		else if (flag == SINGLE_QUOTE)
			flag = state_singlequote(term_i, flag);
		else if (flag == DOUBLE_QUOTE)
			flag = state_doublequote(term_i, flag);
		else
			(*term_i)++;
	}
	(*new_token)->value = ft_substr(start, 0, *term_i - start);
	if (flag != NO_QUOTE && flag != BREAK)
	{
		if (flag == SINGLE_QUOTE)
			(*new_token)->syntax_error = SYNTAX_SINGLE_QUOTE;
		if (flag == DOUBLE_QUOTE)
			(*new_token)->syntax_error = SYNTAX_DOUBLE_QUOTE;
	}
	(*new_token)->type = TOKEN_WORD;
}

int	state_noquote(char **term_i, int flag)
{
	while ((ft_isprint(**term_i) && !ft_isspace(**term_i)
			&& !ft_isoperator(**term_i) && flag == NO_QUOTE))
	{
		flag = state_switch(*term_i, flag);
		if (flag != NO_QUOTE)
		{
			(*term_i)++;
			break ;
		}
		(*term_i)++;
	}
	if (ft_isspace(**term_i) || ft_isoperator(**term_i))
		flag = state_switch(*term_i, flag);
	return (flag);
}

int	state_singlequote(char **term_i, int flag)
{
	while ((ft_isprint(**term_i) && flag == SINGLE_QUOTE))
	{
		flag = state_switch(*term_i, flag);
		if (flag != SINGLE_QUOTE)
		{
			(*term_i)++;
			break ;
		}
		(*term_i)++;
	}
	return (flag);
}

int	state_doublequote(char **term_i, int flag)
{
	while ((ft_isprint(**term_i) && flag == DOUBLE_QUOTE))
	{
		flag = state_switch(*term_i, flag);
		if (flag != DOUBLE_QUOTE)
		{
			(*term_i)++;
			break ;
		}
		(*term_i)++;
	}
	return (flag);
}

int	state_switch(char *term_i, int flag)
{
	if (*term_i == '\'' && flag == NO_QUOTE)
		return (SINGLE_QUOTE);
	if (*term_i == '\'' && flag == SINGLE_QUOTE)
		return (NO_QUOTE);
	if (*term_i == '\"' && flag == NO_QUOTE)
		return (DOUBLE_QUOTE);
	if (*term_i == '\"' && flag == DOUBLE_QUOTE)
		return (NO_QUOTE);
	if ((ft_isspace(*term_i) || ft_isoperator(*term_i)) && flag == NO_QUOTE)
		return (BREAK);
	return (flag);
}
