/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_struct.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/17 22:43:36 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/01 18:34:26 by mnieto-m         ###   ########.fr       */
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


void	ft_vector_init(t_vector *vector, size_t elem_size);
void	ft_vector_free(t_vector *vector);
int	ft_vector_push(t_vector *vector, const void *elem);
void	*ft_vector_get(t_vector *vector, size_t index);

#endif