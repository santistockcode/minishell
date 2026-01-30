/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   assing_lexing_list.c                               :+:      :+:    :+:   */
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
** ASSIGN_REDIR_TOKEN - Detecta y asigna redirecciones
** ============================================================================
**
** DESCRIPCIÓN:
**   Identifica redireccionamientos: <<, >>, <, >
**   Verifica 2 caracteres primero (<<, >>) antes de 1 carácter (<, >)
**
** PARÁMETROS:
**   - current: Puntero a carácter actual (se avanza)
**   - new_token: Token a rellenar
**
** TIPOS DETECTADOS:
**   - "<<" → TOKEN_HEREDOC
**   - ">>" → TOKEN_REDIR_APPEND
**   - "<"  → TOKEN_REDIR_IN
**   - ">"  → TOKEN_REDIR_OUT
**
** AVANCE DEL PUNTERO:
**   - 2 caracteres para <<, >>
**   - 1 carácter para <, >
**
** EJEMPLO:
**   ">> file" → TOKEN_REDIR_APPEND, current avanza 2
**   "> file"  → TOKEN_REDIR_OUT, current avanza 1
** ============================================================================
*/

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

/*
** ============================================================================
** ASSIGN_PIPEVAR_TOKEN - Detecta y asigna pipes
** ============================================================================
**
** DESCRIPCIÓN:
**   Identifica el carácter pipe '|'
**
** PARÁMETROS:
**   - current: Puntero a carácter actual (se avanza 1)
**   - new_token: Token a rellenar
**
** TIPO DETECTADO:
**   - "|" → TOKEN_PIPE
**
** EJEMPLO:
**   "| grep" → TOKEN_PIPE, current avanza 1
** ============================================================================
*/

void	assign_pipevar_token(char **current, t_token **new_token)
{
	if (ft_strncmp(*current, "|", 1) == 0)
	{
		(*new_token)->value = ft_substr(*current, 0, 1);
		(*new_token)->type = TOKEN_PIPE;
		(*current)++;
	}
}

/*
** ============================================================================
** ASSIGN_VAR_TOKEN - Reconoce y marca variables de asignación
** ============================================================================
**
** DESCRIPCIÓN:
**   Detecta si un TOKEN_WORD es realmente una asignación VAR=valor.
**   Valida que el nombre sea alfanumérico + underscore.
**
** PARÁMETROS:
**   - token: Token que ya tiene value (debe ser TOKEN_WORD)
**
** PROCESO:
**   1. Buscar '=' en token->value
**   2. Si no hay '=' → retorna (no es asignación)
**   3. Extraer nombre (parte antes de '=')
**   4. Validar nombre:
**      - Contiene comillas → NO válido (flag = 1)
**      - Contiene caracteres no alfanuméricos (_ sí) → NO válido
**   5. Si válido → type = TOKEN_ASSIGN_WORD
**
** VALIDACIÓN:
**   - VAR=valor → válido → TOKEN_ASSIGN_WORD
**   - VAR NAME=valor → NO válido (NO empieza con =)
**   - 'VAR'=valor → NO válido (contiene comillas)
**   - VAR-NAME=valor → NO válido (contiene -)
**
** EJEMPLO:
**   "PATH=/usr/bin" → TOKEN_ASSIGN_WORD
**   "MY_VAR=123"    → TOKEN_ASSIGN_WORD
**   "echo hello"    → TOKEN_WORD (no tiene =)
** ============================================================================
*/

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

	/* Validar que el nombre sea alfanumérico + underscore */
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

/*
** ============================================================================
** ASSIGN_WORD_TOKEN - Procesa palabras respetando comillas
** ============================================================================
**
** DESCRIPCIÓN:
**   Lee una palabra completa usando máquina de estados para comillas.
**   Soporta: sin comillas, comilla simple (literal), comilla doble.
**
** PARÁMETROS:
**   - current: Puntero a inicio de palabra (se avanza)
**   - new_token: Token a rellenar
**
** MÁQUINA DE ESTADOS:
**   - NO_QUOTE (0): Sin comillas, detecta ' " y operadores
**   - SINGLE_QUOTE (1): Dentro ' ' - todo literal
**   - DOUBLE_QUOTE (2): Dentro " " - todo literal
**   - BREAK (4): Fin de palabra (espacio u operador)
**
** ERRORES DE SINTAXIS:
**   - Comilla simple sin cerrar → syntax_error = SYNTAX_SINGLE_QUOTE
**   - Comilla doble sin cerrar → syntax_error = SYNTAX_DOUBLE_QUOTE
**
** PROCESO:
**   1. Guardar start = *current
**   2. Bucle: cambiar estado según carácter
**   3. Cuando flag == BREAK, extraer substring (start a current)
**   4. Asignar type = TOKEN_WORD
**
** EJEMPLO:
**   "hello world" → TOKEN_WORD: "hello", current → espacio
**   "'hello world'" → TOKEN_WORD: "'hello world'"
**   "he'llo wor'ld" → TOKEN_WORD: "he'llo wor'ld"
** ============================================================================
*/

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

	/* Extraer substring desde start hasta current */
	(*new_token)->value = ft_substr(start, 0, *current - start);

	/* Marcar errores de comillas sin cerrar */
	if (flag != NO_QUOTE && flag != BREAK)
	{
		if (flag == SINGLE_QUOTE)
			(*new_token)->syntax_error = SYNTAX_SINGLE_QUOTE;
		if (flag == DOUBLE_QUOTE)
			(*new_token)->syntax_error = SYNTAX_DOUBLE_QUOTE;
	}

	(*new_token)->type = TOKEN_WORD;
}
