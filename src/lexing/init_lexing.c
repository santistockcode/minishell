/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_lexing.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 23:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 12:24:40 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

int	init_new_token(t_token **new_token, int token_id)
{
	*new_token = ft_calloc(sizeof(t_token), 1);
	if (!*new_token)
		return (MALLOC_ERROR);
	(*new_token)->id = token_id;
	(*new_token)->type = TOKEN_EOF;
	(*new_token)->value = NULL;
	(*new_token)->syntax_error = SYNTAX_OK;
	return (SUCCESS);
}

/*
** ============================================================================
** PROCESS_ASSIGN_TOKEN - Procesa un token individual para asignación
** ============================================================================
**
** DESCRIPCIÓN:
**   Verifica si un token debe ser convertido a TOKEN_ASSIGN_WORD
**
** PARÁMETROS:
**   - token: Token actual a procesar
**   - prev_node: Nodo previo en la lista (NULL si es el primero)
**
** LÓGICA:
**   - Si no hay nodo previo y es WORD → llama assign_var_token
**   - Si hay previo y no es HEREDOC y es WORD → llama assign_var_token
** ============================================================================
*/

static void	process_assign_token(t_token *token, t_list *prev_node)
{
	t_token	*prev;

	if (!token)
		return ;
	if (!prev_node)
	{
		if (token->type == TOKEN_WORD)
			assign_var_token(token);
	}
	else
	{
		prev = (t_token *)prev_node->content;
		if (prev && prev->type != TOKEN_HEREDOC && token->type == TOKEN_WORD)
			assign_var_token(token);
	}
}

/*
** ============================================================================
** REVAL_ASSIGN_TOKEN - Reasigna tipos para variables de asignación
** ============================================================================
**
** DESCRIPCIÓN:
**   Itera lista de tokens y reconoce patrones VAR=valor.
**   Cambia tipo de TOKEN_WORD a TOKEN_ASSIGN_WORD si cumple reglas.
**
** PARÁMETROS:
**   - tokens: Lista de tokens ya procesados
**
** REGLAS DE ASIGNACIÓN:
**   - Debe estar en posición inicial (token 0) O
**   - Anterior no es TOKEN_HEREDOC
**   - Formato: nombre=valor donde nombre es alfanumérico + _
**
** EJEMPLO:
**   VÁLIDO: VAR=valor (inicio)
**   VÁLIDO: echo VAR=valor (después de palabra)
**   INVÁLIDO: << VAR=valor (después de heredoc)
** ============================================================================
*/

void	reval_assign_token(t_list *tokens)
{
	t_token	*token;
	t_list	*current;
	t_list	*prev_node;

	if (!tokens)
		return ;
	current = tokens;
	prev_node = NULL;
	while (current)
	{
		token = (t_token *)current->content;
		if (token)
			process_assign_token(token, prev_node);
		prev_node = current;
		current = current->next;
	}
}

int	syntax_quotes(t_list *tokens)
{
	t_token	*token;
	t_list	*current;

	current = tokens;
	while (current)
	{
		token = (t_token *)current->content;
		if (token && token->syntax_error != SYNTAX_OK)
			return (INPUT_ERROR);
		current = current->next;
	}
	return (SUCCESS);
}
