/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexing_list.c                                      :+:      :+:    :+:   */
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
** GET_TOKENS_LIST - Procesa buffer y crea lista de tokens
** ============================================================================
** 
** DESCRIPCIÓN:
**   Itera sobre lexing_buff (de readline) y transforma cada elemento
**   en un token, almacenándolo en una lista enlazada.
**
** PARÁMETROS:
**   - lexing: Estructura que contiene:
**     * buff: Buffer de entrada (lexing_buff de shell)
**     * tokens: Lista donde se almacenarán los tokens
**
** RETORNA:
**   - SUCCESS: Procesamiento exitoso
**   - INPUT_ERROR: Error durante procesamiento
**   - MALLOC_ERROR: Error de asignación de memoria
**
** FLUJO:
**   1. Inicializa current al inicio del buffer
**   2. while (*current) → itera hasta '\0'
**   3. Salta espacios en blanco
**   4. Llama add_token_list() para crear cada token
**   5. Añade token EOF con siguient ID
**
** EJEMPLO:
**   Entrada: "ls -l | grep test"
**   Tokens: [ls] [-l] [|] [grep] [test] [EOF]
** ============================================================================
*/

int	get_tokens_list(t_lexing *lexing)
{
	t_token	*token_eof;

	lexing->current = lexing->buff;
	lexing->token_id = 0;
	lexing->tokens = NULL;

	/* Itera sobre todo el buffer */
	while (*(lexing->current))
	{
		/* Salta espacios en blanco */
		if (ft_isspace(*(lexing->current)))
		{
			(lexing->current)++;
			continue ;
		}

		/* Crea token en la lista */
		if (add_token_list(lexing, &(lexing->current)) != SUCCESS)
			return (INPUT_ERROR);

		(lexing->token_id)++;
	}

	/* Añade token EOF al final */
	token_eof = NULL;
	if (init_new_token(&token_eof, lexing->token_id) != SUCCESS)
		return (MALLOC_ERROR);
	ft_lstadd_back(&(lexing->tokens), ft_lstnew(token_eof));

	return (SUCCESS);
}

/*
** ============================================================================
** ADD_TOKEN_LIST - Crea un token y lo añade a la lista
** ============================================================================
**
** DESCRIPCIÓN:
**   Crea una nueva estructura t_token, detecta su tipo mediante
**   token_switch(), y lo agrega a la lista enlazada.
**
** PARÁMETROS:
**   - lexing: Contexto con ID y lista de destino
**   - current: Puntero a posición actual (se modifica según token)
**
** RETORNA:
**   - SUCCESS: Token creado y añadido
**   - MALLOC_ERROR: Error de memoria
**   - INPUT_ERROR: Error en token_switch()
**
** PASOS INTERNOS:
**   1. Malloc de t_token con init_new_token()
**   2. token_switch() detecta tipo (<>, |, palabra)
**   3. token_switch() llena token->value
**   4. Crear nodo de lista (ft_lstnew)
**   5. Añadir a lista (ft_lstadd_back)
**
** NOTA IMPORTANTE:
**   - El puntero 'current' se avanza DENTRO de assign_*_token()
**   - Eso permite leer múltiples caracteres según tipo (<<, >>, etc.)
** ============================================================================
*/

int	add_token_list(t_lexing *lexing, char **current)
{
	t_token	*new_token;
	t_list	*new_node;

	new_token = NULL;
	if (init_new_token(&new_token, lexing->token_id) != SUCCESS)
		return (MALLOC_ERROR);

	/* token_switch detecta tipo y rellena value */
	if (token_switch(new_token, current) != SUCCESS)
	{
		free(new_token->value);
		free(new_token);
		return (INPUT_ERROR);
	}

	/* Crear nodo de la lista y agregarlo */
	new_node = ft_lstnew(new_token);
	if (!new_node)
	{
		free(new_token->value);
		free(new_token);
		return (MALLOC_ERROR);
	}

	ft_lstadd_back(&(lexing->tokens), new_node);
	return (SUCCESS);
}

/*
** ============================================================================
** TOKEN_SWITCH - Máquina de cambio de tipo de token
** ============================================================================
**
** DESCRIPCIÓN:
**   Examina el primer carácter y determina qué función de asignación
**   llamar. Cada una es especializada en su tipo de token.
**
** PARÁMETROS:
**   - new_token: Token a rellenar
**   - current: Puntero a carácter actual (se modifica)
**
** LÓGICA:
**   if (**current es '<' o '>')
**      → assign_redir_token() [<<, >>, <, >]
**   else if (**current es '|')
**      → assign_pipevar_token() [|]
**   else if (**current es imprimible)
**      → assign_word_token() [palabras con comillas]
**
** VALIDACIÓN:
**   Si new_token->value sigue NULL, malloc falló
**
** EJEMPLO:
**   '>' → assign_redir_token() → TOKEN_REDIR_OUT: ">"
**   '|' → assign_pipevar_token() → TOKEN_PIPE: "|"
**   'l' → assign_word_token() → TOKEN_WORD: "ls"
** ============================================================================
*/

int	token_switch(t_token *new_token, char **current)
{
	if (ft_strchr("<>", **current))
		assign_redir_token(current, &new_token);
	else if (ft_strchr("|", **current))
		assign_pipevar_token(current, &new_token);
	else if (ft_isprint(**current))
		assign_word_token(current, &new_token);

	if (!new_token->value)
		return (MALLOC_ERROR);

	return (SUCCESS);
}

/*
** ============================================================================
** LEXING - Punto de entrada principal desde shell
** ============================================================================
**
** DESCRIPCIÓN:
**   Función pública que se llama desde main/REPL.
**   Coordina todo el proceso: tokenización, validación, reasignación.
**
** PARÁMETROS:
**   - minishell: Estructura principal de shell
**     REQUISITOS:
**     * minishell->lexing_buff debe contener entrada de readline
**     * minishell->tokens_list será sobrescrito con nueva lista
**
** RETORNA:
**   - SUCCESS: Análisis lexicográfico completo
**   - INPUT_ERROR: Errores en tokens o sintaxis
**
** PASOS:
**   1. Validar entrada
**   2. Crear estructura t_lexing
**   3. Procesar tokens → get_tokens_list()
**   4. Validar comillas → syntax_quotes()
**   5. Reasignar VAR=valor → reval_assign_token()
**   6. Guardar resultado en shell
**
** FLUJO COMPLETO:
**   readline() → lexing_buff → lexing() → tokens_list → parser()
** ============================================================================
*/

int	lexing(t_shell *minishell)
{
	t_lexing	lexing_ctx;

	if (!minishell || !minishell->lexing_buff)
		return (INPUT_ERROR);

	/* Inicializar contexto de lexing */
	lexing_ctx.buff = minishell->lexing_buff;
	lexing_ctx.current = NULL;
	lexing_ctx.token_id = 0;
	lexing_ctx.tokens = NULL;

	/* Procesar tokens */
	if (get_tokens_list(&lexing_ctx) != SUCCESS)
		return (INPUT_ERROR);

	/* Verificar sintaxis de comillas */
	if (syntax_quotes(lexing_ctx.tokens) != SUCCESS)
	{
		free_tokens_list(lexing_ctx.tokens);
		return (INPUT_ERROR);
	}

	/* Reasignar variables de asignación (VAR=valor) */
	reval_assign_token(lexing_ctx.tokens);

	/* Guardar tokens en shell para siguiente fase */
	minishell->tokens_list = lexing_ctx.tokens;

	return (SUCCESS);
}
