/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   set_exec.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 12:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 12:39:39 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SET_EXEC_H
# define SET_EXEC_H

# include "minishell.h"
# include "token_struct.h"
# include "exec.h"

typedef struct s_shell		t_shell;
typedef struct s_command	t_command;
typedef struct s_cmd		t_cmd;
typedef struct s_list		t_list;

/**
 * set_to_exec() - Convert parsing structures to execution structures
 * @shell: Shell context with commands list
 *
 * Converts shell->commands (list of t_command) into a new list of t_cmd
 * ready for execution. Stores result in shell->exec_cmds.
 * Returns SUCCESS on success, error code otherwise.
 */
int		set_to_exec(t_shell *shell);

/**
 * convert_command_to_cmd() - Convert single t_command to t_cmd
 * @command: Parsing command structure
 *
 * Creates a t_cmd with argv and redirs from the parsing structure.
 * Returns pointer to new t_cmd or NULL on error.
 */
t_cmd	*convert_command_to_cmd(t_command *command);

/**
 * build_argv() - Build argv array from command structure
 * @command: Parsing command structure
 *
 * Combines cmd_word and suffix words into a NULL-terminated array.
 * Returns pointer to argv array or NULL on error.
 */
char	**build_argv(t_command *command);

/**
 * build_redirs() - Build redirections list from command structure
 * @command: Parsing command structure
 *
 * Extracts redirections from prefix and suffix, converts to t_redir format.
 * Returns list of t_redir* or NULL if no redirections.
 */
t_list	*build_redirs(t_command *command);

/**
 * convert_io_type() - Convert parsing redirect type to exec redirect type
 * @io_type: Parsing redirection type
 *
 * Maps t_io_redirect_type to t_redir_type.
 * Returns corresponding t_redir_type.
 */
t_redir_type	convert_io_type(t_io_redirect_type io_type);

/**
 * count_argv_size() - Count total arguments in command
 * @command: Parsing command structure
 *
 * Counts cmd_word (if present) + all word suffixes (excluding redirections).
 * Returns total argument count.
 */
int		count_argv_size(t_command *command);

/**
 * add_redir_from_prefix() - Extract redirections from prefix list
 * @prefix_list: List of t_prefix nodes
 * @redirs: Output list to append redirections to
 *
 * Returns SUCCESS or error code.
 */
int		add_redir_from_prefix(t_list *prefix_list, t_list **redirs);

/**
 * add_redir_from_suffix() - Extract redirections from suffix list
 * @suffix_list: List of t_suffix nodes
 * @redirs: Output list to append redirections to
 *
 * Returns SUCCESS or error code.
 */
int		add_redir_from_suffix(t_list *suffix_list, t_list **redirs);

/**
 * create_redir() - Create a t_redir structure
 * @io_redir: Parsing IO redirect structure
 *
 * Creates and initializes a t_redir from parsing io_redirect.
 * Returns pointer to new t_redir or NULL on error.
 */
t_redir	*create_redir(t_io_redirect *io_redir);

#endif
