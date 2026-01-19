/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_stage.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 18:17:59 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/19 19:44:04 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void		safe_close_p(int *p);
void		ft_split_free(char **paths);
const char	*get_path_envp(t_list *env);

char	*msh_path_from_cmdname(char *arg, t_list *env, t_shell *sh)
{
	char		**paths;
	char		*path;
	char		*bar;
	char		**paths_start;
	const char	*env_value;

	env_value = get_path_envp(env);
	if (!env_value || !(*env_value))
		return (NULL);
	paths = ft_split(env_value, ':');
	paths_start = paths;
	while (paths && *paths)
	{
		bar = ft_strjoin(*paths, "/");
		path = ft_strjoin(bar, arg);
		free(bar);
		if (access(path, F_OK) == 0)
			return (msh_set_error(sh, ACCESS_OP), ft_split_free(paths_start),
				path);
		free(path);
		paths++;
	}
	ft_split_free(paths_start);
	return (NULL);
}

char	*msh_resolve_path(char **args, t_list *envp, t_shell *sh)
{
	char	*path;

	if (!args || !args[0] || args[0][0] == '\0')
		return (NULL);
	if (access(args[0], 0) == 0)
		path = ft_strdup(args[0]);
	else
		path = msh_path_from_cmdname(args[0], envp, sh);
	dprintf(STDERR_FILENO, "[msh_resolve_path]: %s\n", path);
	return (path);
}

char *const	*envp_from_env_list(t_list *env)
{
	t_env	*env_var;
	char	**envp;
	char	*pair;
	int		size;
	int		i;

	size = ft_lstsize(env);
	envp = (char **)malloc(sizeof(char *) * (size + 1));
	if (!envp)
		return (NULL);
	i = 0;
	while (env)
	{
		env_var = (t_env *)env->content;
		if (env_var)
		{
			pair = ft_strjoin(env_var->key, "=");
			envp[i] = ft_strjoin(pair, env_var->value);
			free(pair);
			i++;
		}
		env = env->next;
	}
	envp[i] = NULL;
	return (envp);
}

// Entry point for execution
int	msh_exec_stage(t_shell *sh, t_cmd *cmd, t_list *env, int *p)
{
	const t_stage_io	*rdr_spec;
	char				*path;
	int					st;

	rdr_spec = cmd->stage_io;
	if (rdr_spec->in_fd != -1)
		if (dup2_wrap(rdr_spec->in_fd, STDIN_FILENO) == -1)
			return (safe_close_p(p), -1);
	if (rdr_spec->out_fd != -1)
		if (dup2_wrap(rdr_spec->out_fd, STDOUT_FILENO) == -1)
			return (safe_close_p(p), -1);
	safe_close_p(p);
	path = msh_resolve_path(cmd->argv, env, sh);
	if (!path)
		return (exit_status = 127, (-1));
	if (execve_wrap(path, cmd->argv, envp_from_env_list(env)) == -1)
	{
		dprintf(STDERR_FILENO, "[msh_exec_stage]: execve failed\n");
		st = msh_status_from_execve_error(errno);
		msh_set_error(sh, EXECVE_OP);
		msh_print_last_error(sh);
		exit(st);
	}
	free(path);
	exit(EXIT_FAILURE); // TODDO: pending testing, when does code reaches here?
}
