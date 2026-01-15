#ifndef ENVP_H
# define ENVP_H

#include "minishell.h"

typedef struct s_env
{
     char            *key;
     char            *value;
}   t_env;

// INIT

t_list *init_envp(char **envp);
t_env *init_node(char *str);

//PRINT VALUES
void	print_env_list(t_list *env);

//FREES
void free_env(t_env *aux);
void free_list(t_list **env);

//EXPORT

//UNSET

#endif