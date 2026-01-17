#ifndef ENVP_H
# define ENVP_H

typedef struct s_shell	t_shell;
typedef struct s_list	t_list;
typedef struct s_env	t_env;

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