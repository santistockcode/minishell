/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 18:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/09 20:37:33 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXPAND_H
# define EXPAND_H

# include "minishell.h"

int		expand_variables(t_shell *shell);
int		expand_command(t_command *cmd, t_list *env);
int		expand_prefix_list(t_list *prefix_list, t_list *env);
int		expand_suffix_list(t_list *suffix_list, t_list *env);
int		expand_and_quotes(char **string, t_list *env);
int		expand_var_value(char **string, t_list *env);
int		expand_and_replace(char **value, char **start, t_list *env);
int		expand_noquote(char **value, char **start, int flag, t_list *env);
int		expand_singlequote(char **value, int flag);
int		expand_doublequote(char **value, char **start, int flag, t_list *env);

char	*extract_varname(char *value);
char	*extract_varvalue(char *var_name, t_list *env);
char	*expand_varstr(char *string, char *value, char **var_param,
			size_t *value_offset);
char	*find_var_match(char *string, char *value, char *var_name,
			size_t *offset);
char	*join_parts(char *before, char *var_val, char *after);
int		valid_varname(int c);

int		remove_string_quotes(char **string);
int		remove_noquote(char **value, char **start, int flag);
int		remove_singlequote(char **value, char **start, int flag);
int		remove_doublequote(char **value, char **start, int flag);
int		remove_char_quote(char **start, char **value);

#endif
