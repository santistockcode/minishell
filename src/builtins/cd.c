/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 16:21:19 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/09 20:41:29 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

/*
Spec:
	- si hay más de 1 argumento minishell: cd: too many arguments
	- si no se pasan argumentos o pasan virgulilla:
			- si home no está seteado: minishell: cd: HOME not set y return 1
			-hacemos chdir a HOME
			- return 0
	- si el argumento es "-"
			- si OLDPWD no está seteado: minishell: cd: OLDPWD not set y return 1
			-hacemos chdir a OLDPW
		- imprime PWD
		- return 0
	- si al hacer chdir devuelve
		-1: minishell: cd: <path>: No such file or directory
	- antes de salir de cualquiera de los 3 casos anteriores actualizamos env:
		- getcwd para ver donde estoy
		- setenv("OLDPWD", cwd, 1);
		- setenv("PWD", new_cwd, 1);
	- liberamoos lo que haga falta
*/
t_env	*env_get(t_list *env, char *key);
int		export(t_list **env, char *var);

int	print_error_cd_malloc(void)
{
	ft_putstr_fd("minishell: cd: malloc failure\n", STDERR_FILENO);
	return (1);
}

int	update_pwd(t_list **env)
{
	char	*cwd;
	t_env	*ev_oldpwd;
	char	*joinpwd;
	char	*joinoldpwd;

	cwd = getcwd(NULL, 0);
	ev_oldpwd = env_get(*env, "PWD");
	if (ev_oldpwd)
	{
		joinoldpwd = ft_strjoin("OLDPWD=", ev_oldpwd->value);
		if (!joinoldpwd)
			return (free(cwd), print_error_cd_malloc());
		export(env, joinoldpwd);
		free(joinoldpwd);
	}
	joinpwd = ft_strjoin("PWD=", cwd);
	if (!joinpwd)
		return (free(cwd), print_error_cd_malloc());
	export(env, joinpwd);
	free(joinpwd);
	free(cwd);
	return (0);
}

int	handle_cd_home(t_list **env)
{
	t_env	*ev_home;

	ev_home = env_get(*env, "HOME");
	if (!ev_home)
	{
		ft_putstr_fd("minishell: cd: HOME not set\n", STDERR_FILENO);
		return (1);
	}
	if (chdir(ev_home->value) == -1)
	{
		ft_putstr_fd("minishell: cd: ", STDERR_FILENO);
		ft_putstr_fd(ev_home->value, STDERR_FILENO);
		ft_putstr_fd(": No such file or directory\n", STDERR_FILENO);
		return (1);
	}
	update_pwd(env);
	return (0);
}

int	handle_cd_oldpwd(t_list **env)
{
	t_env	*oldpwd;

	oldpwd = env_get(*env, "OLDPWD");
	if (!oldpwd)
	{
		ft_putstr_fd("minishell: cd: OLDPWD not set\n", STDERR_FILENO);
		return (1);
	}
	if (chdir(oldpwd->value) == -1)
	{
		ft_putstr_fd("minishell: cd: ", STDERR_FILENO);
		ft_putstr_fd(oldpwd->value, STDERR_FILENO);
		ft_putstr_fd(": No such file or directory\n", STDERR_FILENO);
		return (1);
	}
	ft_putstr_fd(oldpwd->value, STDOUT_FILENO);
	return (update_pwd(env));
}

/*
FIXME: cd, ~/a/b should concatenate $HOME to <path> (expand)
*/
int	cd_builtin(char **argsv, t_list **env)
{
	if (argsv[1] && argsv[2])
	{
		ft_putstr_fd("minishell: cd: too many arguments\n", STDERR_FILENO);
		return (1);
	}
	if (!argsv[1] || (argsv[1] && (ft_strncmp(argsv[1], "~", 1) == 0)))
		return (handle_cd_home(env));
	if (ft_strncmp(argsv[1], "-", 1) == 0)
		return (handle_cd_oldpwd(env));
	if (chdir(argsv[1]) == -1)
	{
		ft_putstr_fd("minishell: cd: ", STDERR_FILENO);
		ft_putstr_fd(argsv[1], STDERR_FILENO);
		ft_putstr_fd(": No such file or directory\n", STDERR_FILENO);
		return (1);
	}
	return (update_pwd(env));
}
