/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   automa_lexing_list.c                               :+:      :+:    :+:   */
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
** MÁQUINA DE ESTADOS PARA PROCESAMIENTO DE PALABRAS
** ============================================================================
**
** DESCRIPCIÓN:
**   Sistema de 3 estados para reconocer palabras respetando comillas.
**   
** ESTADOS:
**   0 = NO_QUOTE (sin comillas): Lee caracteres normales hasta espacio/operador
**   1 = SINGLE_QUOTE ('....'): Lee TODO literalmente hasta '
**   2 = DOUBLE_QUOTE ("....'): Lee TODO literalmente hasta "
**   4 = BREAK: Fin de palabra
**
** TRANSICIONES:
**   NO_QUOTE + ' → SINGLE_QUOTE (entra comilla simple)
**   NO_QUOTE + " → DOUBLE_QUOTE (entra comilla doble)
**   SINGLE_QUOTE + ' → NO_QUOTE (sale comilla simple)
**   DOUBLE_QUOTE + " → NO_QUOTE (sale comilla doble)
**   NO_QUOTE + espacio/operador → BREAK (fin)
**
** FUNCIÓN ORQUESTADORA:
**   assign_word_token() elige qué estado_* llamar
** ============================================================================
*/

/*
** ============================================================================
** STATE_NOQUOTE - Estado sin comillas
** ============================================================================
**
** DESCRIPCIÓN:
**   Lee caracteres imprimibles que no sean espacios ni operadores.
**   Si encuentra ' o ", cambia de estado.
**   Si encuentra espacio u operador, termina (BREAK).
**
** PARÁMETROS:
**   - current: Puntero a carácter (se avanza)
**   - flag: Estado actual
**
** LÓGICA:
**   while (carácter imprimible Y no espacio Y no operador Y flag==NO_QUOTE)
**   {
**       flag = state_switch() → verifica si hay comilla/operador
**       if (cambió de estado)
**           avanzar y salir
**       else
**           avanzar
**   }
**   if (espacio o operador)
**       flag = state_switch() → BREAK
**
** EJEMPLO:
**   "hello world" → lee "hello", current → espacio
**   "he'llo" → lee "he", flag→SINGLE_QUOTE
** ============================================================================
*/

int	state_noquote(char **current, int flag)
{
	while ((ft_isprint(**current) && !ft_isspace(**current)
			&& !ft_isoperator(**current) && flag == NO_QUOTE))
	{
		flag = state_switch(*current, flag);
		if (flag != NO_QUOTE)
		{
			(*current)++;
			break ;
		}
		(*current)++;
	}
	if (ft_isspace(**current) || ft_isoperator(**current))
		flag = state_switch(*current, flag);
	return (flag);
}

/*
** ============================================================================
** STATE_SINGLEQUOTE - Estado dentro de comillas simples
** ============================================================================
**
** DESCRIPCIÓN:
**   Lee TODO literalmente hasta encontrar comilla simple de cierre.
**   Adentro, incluso " y \ se tratan como caracteres normales.
**
** PARÁMETROS:
**   - current: Puntero a carácter (se avanza)
**   - flag: Estado actual (debe ser SINGLE_QUOTE)
**
** LÓGICA:
**   while (carácter imprimible Y flag==SINGLE_QUOTE)
**   {
**       if (carácter es ')
**           flag = NO_QUOTE (sale)
**           avanzar y salir
**       else
**           avanzar
**   }
**
** EJEMPLO:
**   'hello world' → lee todo literalmente, sale con '
**   'can't do' → lee "can't do" (la ' interna no cierra)
** ============================================================================
*/

int	state_singlequote(char **current, int flag)
{
	while ((ft_isprint(**current) && flag == SINGLE_QUOTE))
	{
		flag = state_switch(*current, flag);
		if (flag != SINGLE_QUOTE)
		{
			(*current)++;
			break ;
		}
		(*current)++;
	}
	return (flag);
}

/*
** ============================================================================
** STATE_DOUBLEQUOTE - Estado dentro de comillas dobles
** ============================================================================
**
** DESCRIPCIÓN:
**   Lee TODO literalmente hasta encontrar comilla doble de cierre.
**   Similar a SINGLE_QUOTE pero para ".
**
** PARÁMETROS:
**   - current: Puntero a carácter (se avanza)
**   - flag: Estado actual (debe ser DOUBLE_QUOTE)
**
** LÓGICA:
**   while (carácter imprimible Y flag==DOUBLE_QUOTE)
**   {
**       if (carácter es ")
**           flag = NO_QUOTE (sale)
**           avanzar y salir
**       else
**           avanzar
**   }
**
** EJEMPLO:
**   "hello world" → lee todo, sale con "
**   "can't" → lee completo (comilla simple ignorada)
** ============================================================================
*/

int	state_doublequote(char **current, int flag)
{
	while ((ft_isprint(**current) && flag == DOUBLE_QUOTE))
	{
		flag = state_switch(*current, flag);
		if (flag != DOUBLE_QUOTE)
		{
			(*current)++;
			break ;
		}
		(*current)++;
	}
	return (flag);
}

/*
** ============================================================================
** STATE_SWITCH - Máquina de cambio de estados
** ============================================================================
**
** DESCRIPCIÓN:
**   Evalúa un carácter y determina transición de estado.
**   Es el núcleo de la máquina de estados.
**
** PARÁMETROS:
**   - current: Carácter actual (NO se modifica)
**   - flag: Estado actual
**
** TABLA DE TRANSICIONES:
**   
**   CASO 1: ' y flag==NO_QUOTE → return SINGLE_QUOTE
**           (Entra en comilla simple)
**   
**   CASO 2: ' y flag==SINGLE_QUOTE → return NO_QUOTE
**           (Sale de comilla simple)
**   
**   CASO 3: " y flag==NO_QUOTE → return DOUBLE_QUOTE
**           (Entra en comilla doble)
**   
**   CASO 4: " y flag==DOUBLE_QUOTE → return NO_QUOTE
**           (Sale de comilla doble)
**   
**   CASO 5: espacio/operador y flag==NO_QUOTE → return BREAK
**           (Fin de palabra)
**   
**   DEFAULT: return flag (sin cambios)
**
** EJEMPLO:
**   state_switch('\'', NO_QUOTE) → SINGLE_QUOTE
**   state_switch('\'', SINGLE_QUOTE) → NO_QUOTE
**   state_switch(' ', NO_QUOTE) → BREAK
**   state_switch('a', NO_QUOTE) → NO_QUOTE (sin cambios)
** ============================================================================
*/

int	state_switch(char *current, int flag)
{
	if (*current == '\'' && flag == NO_QUOTE)
		return (SINGLE_QUOTE);
	if (*current == '\'' && flag == SINGLE_QUOTE)
		return (NO_QUOTE);
	if (*current == '\"' && flag == NO_QUOTE)
		return (DOUBLE_QUOTE);
	if (*current == '\"' && flag == DOUBLE_QUOTE)
		return (NO_QUOTE);
	if ((ft_isspace(*current) || ft_isoperator(*current)) && flag == NO_QUOTE)
		return (BREAK);
	return (flag);
}
