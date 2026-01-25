#include "../include/minishell.h"

int	env_set(t_list **env, char *var);
void	env_unset(t_list **env, char *key);

int		is_builtin(char *cmd)
{
	// if (ft_strcmp(cmd, "echo") == 0)
	// 	return (1);
	// if (ft_strcmp(cmd, "cd") == 0)
	// 	return (1);
	// if (ft_strcmp(cmd, "pwd") == 0)
	// 	return (1);
	// if (ft_strcmp(cmd, "env") == 0)
	// 	return (1);
	if (ft_strncmp(cmd, "export", 6) == 0)
		return (1);
	if (ft_strncmp(cmd, "unset", 5) == 0)
		return (1);
	return (0);
}

// TODO: better to treat each builtin as independant programm
// every error has been set, every error has been printed
int		exec_builtin(t_cmd *cmd, t_shell *sh)
{
	int		result;

	result = 0;
	// if (ft_strcmp(args[0], "echo") == 0)
	// 	result = ft_echo(args);
	// if (ft_strcmp(args[0], "cd") == 0)
	// 	result = ft_cd(args, mini->env);
	// if (ft_strcmp(args[0], "pwd") == 0)
	// 	result = ft_pwd();
	// if (ft_strcmp(args[0], "env") == 0)
	// 	ft_env(mini->env);
	if (ft_strncmp(cmd->argv[0], "export", 6) == 0)
		result = env_set(&sh->env, cmd->argv[1]);
	if (ft_strncmp(cmd->argv[0], "unset", 5) == 0)
		env_unset(&sh->env, cmd->argv[1]);
	// FIXME: on error, set op, errno, last_status, print error
	return (result);
}