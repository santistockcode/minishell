/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_struct.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/17 22:43:36 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/03 16:15:18 by mnieto-m         ###   ########.fr       */
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

// Enum Token Type
typedef enum e_io_redirect_type
{
	REDIR_IN, // <
	REDIR_OUT, // >
	REDIR_APPEND, // >>
	HEREDOC, // <<
}	t_io_redirect_type;

typedef struct s_io_file
{
	int		type; // REDIR OUT o REDIR APPEND >> > < 
	char	*filename;
}	t_io_file;

typedef struct s_io_here
{
	char	*filename;
	char	*here_end; // <<
}	t_io_here;

typedef struct s_io_redirect
{
	t_io_file	*io_file;
	t_io_here	*io_here;
}	t_io_redirect;

typedef struct s_prefix
{
	t_io_redirect	*io_redirect;
	char			*assignment_word;
}	t_prefix;

typedef struct s_suffix
{
	t_io_redirect	*io_redirect;
	char			*word;
}	t_suffix;

typedef struct s_command
{
	t_list		*cmd_prefix;	// List of t_prefix nodes
	char		*cmd_word;		// Command name
	t_list		*cmd_suffix;	// List of t_suffix nodes
}	t_command;

#endif