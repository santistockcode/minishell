/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_lexing_list.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 23:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/01/29 23:12:49 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/lexing_new.h"

/*
** ============================================================================
** INIT_NEW_TOKEN - Crea estructura vacía de token
** ============================================================================
**
** DESCRIPCIÓN:
**   Asigna memoria para un t_token y lo inicializa con valores por defecto.
**
** PARÁMETROS:
**   - new_token: Puntero a puntero del token (se rellena)
**   - token_id: ID secuencial (0, 1, 2, ...)
**
** INICIALIZACIÓN:
**   - id: token_id (parámetro)
**   - type: TOKEN_EOF (será sobrescrito por assign_*_token)
**   - value: NULL (será rellenado por assign_*_token)
**   - syntax_error: SYNTAX_OK
**
** RETORNA:
**   - SUCCESS: Token creado correctamente
**   - MALLOC_ERROR: ft_calloc falló
**
** NOTA:
**   Se usa ft_calloc para inicializar todo a 0/NULL automáticamente
** ============================================================================
*/

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
** LÓGICA:
**   1. Iterar lista
**   2. Si es primer token (i==0) y es WORD → call assign_var_token()
**   3. Si no es heredoc y es WORD → call assign_var_token()
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
	t_token	*prev;
	size_t	i;
	t_list	*current;

	if (!tokens)
		return ;

	i = 0;
	current = tokens;
	while (current)
	{
		token = (t_token *)current->content;
		if (!token)
		{
			current = current->next;
			i++;
			continue ;
		}

		if (i == 0)
		{
			if (token->type == TOKEN_WORD)
				assign_var_token(token);
		}
		else
		{
			prev = (t_token *)ft_lstfirst(tokens)->content;
			/* Obtener anterior correctamente */
			for (size_t j = 0; j < i - 1; j++)
				prev = (t_token *)ft_lstfirst(tokens)->next->content;

			if (prev && prev->type != TOKEN_HEREDOC && token->type == TOKEN_WORD)
				assign_var_token(token);
		}
		current = current->next;
		i++;
	}
}

/*
** ============================================================================
** SYNTAX_QUOTES - Valida comillas balanceadas
** ============================================================================
**
** DESCRIPCIÓN:
**   Verifica que no haya comillas sin cerrar.
**
** PARÁMETROS:
**   - tokens: Lista de tokens a validar
**
** RETORNA:
**   - SUCCESS: Sintaxis correcta
**   - INPUT_ERROR: Error de comillas
**
** VALIDACIONES:
**   - TOKEN_WORD con syntax_error != SYNTAX_OK → error
** ============================================================================
*/

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

/*
** ============================================================================
** FREE_TOKENS_LIST - Libera toda la lista de tokens
** ============================================================================
**
** DESCRIPCIÓN:
**   Itera lista y libera cada t_token y su valor.
**
** PARÁMETROS:
**   - tokens: Lista de tokens
**
** PROCESO:
**   1. Iterar cada nodo
**   2. Liberar token->value
**   3. Liberar token
**   4. Liberar nodo (ft_lstclear)
** ============================================================================
*/

void	free_tokens_list(t_list *tokens)
{
	t_list	*current;
	t_list	*next;
	t_token	*token;

	current = tokens;
	while (current)
	{
		next = current->next;
		token = (t_token *)current->content;
		if (token)
		{
			if (token->value)
				free(token->value);
			free(token);
		}
		free(current);
		current = next;
	}
}
