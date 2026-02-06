#include "../../include/minishell.h"


int	env_builtin(t_list	*env)
{
	t_env *current;
    t_list *env_list;

    env_list = env;
	if (!env_list)
		return (EXIT_FAILURE);
	while (env_list)
	{
		current = env_list->content;
		ft_putstr_fd(current->key, STDOUT_FILENO);
		ft_putstr_fd("=", STDOUT_FILENO);
		ft_putstr_fd(current->value, STDOUT_FILENO);
		ft_putstr_fd("\n", STDOUT_FILENO);
		env_list = env_list->next;
	}
	return (EXIT_SUCCESS);
}