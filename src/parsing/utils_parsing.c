/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_parsing.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 20:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/04 18:58:57 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

/**
 * is_io_redirect() - Check if token is an IO redirection
 * @token_type: Type to check
 *
 * Returns token_type if it's a redirection (< > >> <<), 0 otherwise
 */
int	is_io_redirect(t_token_type token_type)
{
	if (token_type >= TOKEN_REDIR_IN && token_type <= TOKEN_HEREDOC)
		return (token_type);
	return (0);
}

/**
 * consume_token() - Move to next token
 * @index: Current position pointer
 */
void	consume_token(int *index)
{
	if (index)
		(*index)++;
}

/**
 * get_token_at() - Get token at specific index in list
 * @tokens: Token list
 * @index: Position to get
 *
 * Returns pointer to t_token at position, or NULL if out of bounds
 */
t_token	*get_token_at(t_list *tokens, int index)
{
	t_list	*current;
	int		count;

	if (!tokens || index < 0)
		return (NULL);
	
	current = tokens;
	count = 0;
	
	while (current && count < index)
	{
		current = current->next;
		count++;
	}
	
	if (!current)
		return (NULL);
	
	return ((t_token *)current->content);
}

/**
 * init_command() - Initialize a command structure
 * @cmd: Output command pointer
 *
 * Returns SUCCESS on success, MALLOC_ERROR on failure
 */
int	init_command(t_command **cmd)
{
	*cmd = ft_calloc(sizeof(t_command), 1);
	if (!*cmd)
		return (MALLOC_ERROR);
	
	(*cmd)->cmd_prefix = NULL;
	(*cmd)->cmd_word = NULL;
	(*cmd)->cmd_suffix = NULL;
	
	return (SUCCESS);
}

/**
 * init_prefix() - Initialize a prefix structure
 * @prefix: Output prefix pointer
 *
 * Returns SUCCESS on success, MALLOC_ERROR on failure
 */
int	init_prefix(t_prefix **prefix)
{
	*prefix = ft_calloc(sizeof(t_prefix), 1);
	if (!*prefix)
		return (MALLOC_ERROR);
	
	(*prefix)->assignment_word = NULL;
	(*prefix)->io_redirect = NULL;
	
	return (SUCCESS);
}

/**
 * init_suffix() - Initialize a suffix structure
 * @suffix: Output suffix pointer
 *
 * Returns SUCCESS on success, MALLOC_ERROR on failure
 */
int	init_suffix(t_suffix **suffix)
{
	*suffix = ft_calloc(sizeof(t_suffix), 1);
	if (!*suffix)
		return (MALLOC_ERROR);
	
	(*suffix)->word = NULL;
	(*suffix)->io_redirect = NULL;
	
	return (SUCCESS);
}
