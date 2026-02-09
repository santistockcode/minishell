/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 18:06:55 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/09 20:11:30 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENVP_H
# define ENVP_H

typedef struct s_shell	t_shell;
typedef struct s_list	t_list;
typedef struct s_env	t_env;

// INIT

t_list					*init_envp(char **envp);
t_env					*init_node(char *str);

// PRINT VALUES
void					print_env_list(t_list *env);

// FREES
void					free_env(t_env *aux);
void					free_list(t_list **env);

// EXPORT
t_env					*env_get(t_list *env, char *key);
int						env_set(t_list **env, char *var);

// UNSET
void					env_unset(t_list **env, char *key);
void					unset(t_list **env, char **argv);
int						wrap_export(t_list **env, char **argv);

#endif