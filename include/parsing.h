/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 15:48:54 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/09 20:36:44 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_H
# define PARSING_H

# include "minishell.h"
# include "token_struct.h"

typedef struct s_shell		t_shell;
typedef struct s_token		t_token;
typedef struct s_command	t_command;
typedef struct s_list		t_list;

/* Main parsing function */
int							parsing(t_shell *shell);

/* Parsing functions */
int							parse_pipe_sequence(t_list **commands,
								t_list *tokens, int *index, t_shell *shell);
int							parse_simple_command(t_command **cmd,
								t_list *tokens, int *index);
int							parse_prefix(t_list **prefix_list, t_list *tokens,
								int *index);
int							parse_suffix(t_list **suffix_list, t_list *tokens,
								int *index);
int							parse_cmd_word(char **cmd_word, t_list *tokens,
								int *index);
int							parse_io_redirect_prefix(t_prefix **prefix,
								t_list *tokens, int *index,
								t_token_type token_type);
int							parse_io_redirect_suffix(t_suffix **suffix,
								t_list *tokens, int *index,
								t_token_type token_type);
int							parse_io_file(t_io_redirect *io_redir,
								t_list *tokens, int *index,
								t_token_type token_type);
int							parse_io_here(t_io_redirect *io_redir,
								t_list *tokens, int *index);

/* Utility functions */
int							is_io_redirect(t_token_type token_type);
t_token						*get_token_at(t_list *tokens, int index);
void						consume_token(int *index);
int							init_command(t_command **cmd);
int							init_prefix(t_prefix **prefix);
int							init_suffix(t_suffix **suffix);

/* Free functions */
void						free_commands(t_list *commands);
void						free_command(t_command *cmd);
void						free_prefixes(t_list *prefix_list);
void						free_suffixes(t_list *suffix_list);

#endif