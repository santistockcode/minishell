/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_struct.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/17 22:43:36 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/01/20 22:54:23 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOKEN_STRUCT_H
# define TOKEN_STRUCT_H

typedef struct s_shell	t_shell;
typedef struct s_list	t_list;
typedef struct s_vector	t_vector;

typedef enum e_token_type
{
	TOKEN_IGNORE, // Whitespace, /, ;, &&, || (no bonus)
	TOKEN_PIPE, // |
	TOKEN_WORD, // A non key word
	TOKEN_REDIR_IN, // <
	TOKEN_REDIR_OUT, // >
	TOKEN_REDIR_APPEND, // >>
	TOKEN_HEREDOC, // <<
	TOKEN_ASSIGN_WORD, // var=value
	TOKEN_EOF, // End of input
}	t_token_type;

typedef enum e_syntax_error
{
	SYNTAX_OK, // All good!!
	SYNTAX_SINGLE_QUOTE, // Missing '
	SYNTAX_DOUBLE_QUOTE, // Missing "
	SYNTAX_INVALID_ASSIGN, // Invalid ASSIGN_WORD
	SYNTAX_BRACES, // Missing ), } or ]
	SYNTAX_OPERATOR, // Incomplete redirection with < << > >>
}	t_syntax_error;

// Individual Token struct
typedef struct s_token
{
	int				id;					// A unique ID for each token
	t_token_type	type;		// The type of token (from enum t_token_type)
	char			*value;			// The string representation of the token
	int				syntax_error;
}	t_token;

// Minishell input
typedef struct s_term_token
{
	char			*term_line;
	t_vector		*token_array;
}	t_term_token;

void	ft_vector_init(t_vector *vector, size_t elem_size);
void	ft_vector_free(t_vector *vector);
int	ft_vector_push(t_vector *vector, const void *elem);
void	*ft_vector_get(t_vector *vector, size_t index);

#endif