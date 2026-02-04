/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 20:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/04 00:03:26 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"
#include "../../include/parsing.h"

/**
 * parsing() - Entry point for parsing phase
 * @shell: Shell context with lexed tokens in shell->lexing->tokens
 *
 * Converts token list into command list using recursive descent parsing
 * Stores result in shell->commands
 * Returns SUCCESS on success, INPUT_ERROR or MALLOC_ERROR on failure
 */
int	parsing(t_shell *shell)
{
	int	index;

	if (!shell || !shell->lexing || !shell->lexing->tokens)
		return (INPUT_ERROR);
	
	index = 0;
	shell->commands = NULL;
	
	if (parse_pipe_sequence(&shell->commands, shell->lexing->tokens,
		&index, shell) != SUCCESS)
	{
		free_commands(shell->commands);
		shell->commands = NULL;
		return (INPUT_ERROR);
	}
	
	return (SUCCESS);
}

/**
 * parse_pipe_sequence() - Parse pipe-separated commands
 * @commands: Output list to append commands to
 * @tokens: Token list from lexing
 * @index: Current position in tokens
 * @shell: Shell context
 *
 * Parses: simple_command ( '|' simple_command )*
 */
int	parse_pipe_sequence(t_list **commands, t_list *tokens,
	int *index, t_shell *shell)
{
	t_command	*cmd;
	t_list		*new_node;
	t_token		*token;

	cmd = NULL;
	if (parse_simple_command(&cmd, tokens, index) != SUCCESS)
		return (INPUT_ERROR);
	if (!cmd)
		return (INPUT_ERROR);
	new_node = ft_lstnew(cmd);
	if (!new_node)
		return (free_command(cmd),MALLOC_ERROR);
	ft_lstadd_back(commands, new_node);
	
	token = get_token_at(tokens, *index);
	if (token && token->type == TOKEN_PIPE)
	{
		consume_token(index);
		if (parse_pipe_sequence(commands, tokens, index, shell) != SUCCESS)
			return (INPUT_ERROR);
	}
	return (SUCCESS);
}

/**
 * parse_simple_command() - Parse a simple command (no pipes)
 * @cmd: Output command structure
 * @tokens: Token list
 * @index: Current position
 *
 * Parses: [prefix] word [suffix]
 */
int	parse_simple_command(t_command **cmd, t_list *tokens,
	int *index)
{
	if (init_command(cmd) != SUCCESS)
		return (MALLOC_ERROR);
	
	if (parse_prefix(&(*cmd)->cmd_prefix, tokens, index) != SUCCESS)
		return (INPUT_ERROR);
	
	if (parse_cmd_word(&(*cmd)->cmd_word, tokens, index) != SUCCESS)
		return (INPUT_ERROR);
	
	if (!(*cmd)->cmd_prefix && !(*cmd)->cmd_word)
	{
		t_token *token = get_token_at(tokens, *index);
		if (token)
			ft_printf("minishell: syntax error near '%s'\n", token->value);
		return (INPUT_ERROR);
	}
	
	if (parse_suffix(&(*cmd)->cmd_suffix, tokens, index) != SUCCESS)
		return (INPUT_ERROR);
	
	return (SUCCESS);
}

/**
 * parse_cmd_word() - Parse command word (first word)
 * @cmd_word: Output word
 * @tokens: Token list
 * @index: Current position
 *
 * Extracts the first WORD token (the command itself)
 */
int	parse_cmd_word(char **cmd_word, t_list *tokens, int *index)
{
	t_token	*token;

	token = get_token_at(tokens, *index);
	if (!token)
		return (INPUT_ERROR);
	
	if (token->type != TOKEN_WORD)
		return (SUCCESS);
	
	*cmd_word = ft_strdup(token->value);
	if (!*cmd_word)
		return (MALLOC_ERROR);
	
	consume_token(index);
	return (SUCCESS);
}