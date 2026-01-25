#include "../include/minishell.h"

void	env_set(t_list **env, char *var);

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
	// if (ft_strcmp(cmd, "unset") == 0)
	// 	return (1);
	return (0);
}

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
		env_set(&sh->env, cmd->argv[1]);
	// if (ft_strcmp(cmd->argv[0], "unset") == 0)
	// 	ft_unset(cmd->argv, sh);
	return (result);
}