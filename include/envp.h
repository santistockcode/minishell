#ifndef ENVP_H
# define ENVP_H
#include "minishell.h"

// INIT
int init_minishell(t_shell **minishell,char **envp);
t_list *init_envp(char **envp);
t_env *init_node(char *str);

//PRINT VALUES
void	print_envp_list(t_list *env);

//FREES
void free_envp(t_env *aux);
void free_list(t_list **env);

//EXPORT

//UNSET

#endif