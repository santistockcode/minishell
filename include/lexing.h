/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexing.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 18:06:38 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/09 20:34:54 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXING_H
# define LEXING_H

# include "minishell.h"

# define NO_QUOTE 0
# define SINGLE_QUOTE 1
# define DOUBLE_QUOTE 2
# define BREAK 4
# define INVALID_STR -1

typedef struct s_token
{
	int				id;
	t_token_type	type;
	char			*value;
	int				syntax_error;
}					t_token;

typedef struct s_lexing
{
	char			*buff;
	char			*current;
	int				token_id;
	t_list			*tokens;
}					t_lexing;

// Funciones públicas
int					get_tokens_list(t_lexing *lexing);
int					add_token_list(t_lexing *lexing, char **current);
int					token_switch(t_token *new_token, char **current);
int					lexing(t_shell *minishell);

// Funciones privadas (lexing)
int					init_new_token(t_token **new_token, int token_id);

// Funciones privadas (asignación de tipos)
void				assign_redir_token(char **current, t_token **new_token);
void				assign_pipevar_token(char **current, t_token **new_token);
void				assign_var_token(t_token *token);
void				assign_word_token(char **current, t_token **new_token);

// Funciones privadas (máquina de estados para palabras)
int					state_noquote(char **current, int flag);
int					state_singlequote(char **current, int flag);
int					state_doublequote(char **current, int flag);
int					state_switch(char *current, int flag);

// Funciones de utilidad
void				reval_assign_token(t_list *tokens);
void				free_tokens_list(t_list *tokens);
int					syntax_quotes(t_list *tokens);
int					ft_isoperator(int c);
void				free_lexing(t_lexing *lexing);
int					not_tokens(t_shell *minishell);

#endif
