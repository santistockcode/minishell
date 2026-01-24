/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_stage.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 18:17:59 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/24 16:44:52 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"


void		safe_close_p(int *p);
void		ft_split_free(char **paths);
const char	*get_path_envp(t_list *env);
char	*ft_strjoin_prot(char const *str1, char const *str2);
void    safe_close_redirs(t_list *redirs);
int		is_builtin(char *cmd);
int		exec_builtin(t_cmd *cmd, t_shell *sh);
void	free_cmd_struct(void *input);
void free_shell_child(t_shell *sh);

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
			pair = ft_strjoin_prot(env_var->key, "=");
			envp[i] = ft_strjoin_prot(pair, env_var->value);
			free(pair);
			i++;
		}
		env = env->next;
	}
	envp[i] = NULL;
	return (envp);
}

char *build_path(const char *dir, const char *file)
{
	char *path;
	char *tmp;

	if (!dir || !file)
		return (NULL);
	tmp = ft_strjoin_prot(dir, "/");
	if (!tmp)
		return (NULL);
	path = ft_strjoin_prot(tmp, file);
	if (!path)
	{
		free(tmp);
		return (NULL);
	}
	free(tmp);
	return (path);
}
/*
Returns: 
- path is the path allocated if found, NULL if not found
- acc_ret is set to 0 on access didn't fail, -1 on syscall error (access failed ENOENT excluded)
- returns -1 if couldn't access, 1 if access succeeded
- ñapa: if malloc fails on libft call: path is NULL, acc_ret is -2, returns -1

MATRIX (path, acc_ret, return value)
path is either allocated_path or NULL
acc_ret is set to -1 -2 0
return value is negative or >= 0

path -1 neg -> ACCESS FAILED
path -1 pos -> not possible
path -2 neg -> not possible
path -2 pos -> not possible
path 0 neg -> PATH NOT FOUND
path 0 pos -> PATH FOUND FOR FILE
NULL -1 neg -> not possible
NULL -1 pos -> not possible
NULL -2 neg -> MALLOC ERROR
NULL -2 pos -> not possible
NULL 0 neg -> not possible
NULL 0 pos -> not possible
*/
int try_access(char **pre_paths, char **path, char *file, int *acc_ret)
{

	if (!pre_paths || !*pre_paths || !file || !path || !acc_ret)
		return (*acc_ret = 0, (-1));
	*path = build_path(*pre_paths, file);
	if (!*path)
		return (*acc_ret = -2, (-1));
	*acc_ret = access_wrap(*path, F_OK);
	if (*acc_ret == -1 && errno != ENOENT)
	{
		free(*path);
		*path = NULL;
		return (-1);
	}
	else if (*acc_ret >= 0)
		return (1);
	*acc_ret = 0;
	free(*path);
	*path = NULL;
	return (-1);
}

/*
Syscall access needs to be controled. 
Should return path on found, NULL on not found.
acc_ret should be set to 0 on success or malloc fail, -1 on error syscall if not ENOENT.
*/
char	*msh_path_from_cmdname(char *arg, t_list *env, t_shell *sh, int *acc_ret)
{
	char		**all_paths;
	char		*path;
	char		**paths_start;
	const char	*env_value;

	env_value = get_path_envp(env);
	if (!env_value || !(*env_value))
		return (*acc_ret = 0, (NULL));
	all_paths = ft_split(env_value, ':');
	if (!all_paths)
		return (*acc_ret = -1, (NULL));
	paths_start = all_paths;
	path = NULL;
	while (all_paths && *all_paths)
	{
		if (try_access(all_paths, &path, arg, acc_ret) >= 0)
			return (ft_split_free(paths_start), path);
		else if (*acc_ret == (-2))
			return (msh_set_error(sh, MALLOC_OP), ft_split_free(paths_start), (NULL));
		else if (*acc_ret == (-1))			
			return (msh_set_error(sh, ACCESS_OP), ft_split_free(paths_start), (NULL));
		all_paths++;
	}
	return (ft_split_free(paths_start), *acc_ret = 0, (NULL)); // TODO: is correct free here when access fails?
}

// returns path on found or NULL on not found and errors (sh->errno already set)
/*
This function resolves the path of a command by checking if it's directly accessible
 or by searching in the PATH environment variable.
*/
char	*msh_resolve_path(char **args, t_list *envp, t_shell *sh)
{
	char	*path;
	int		acc_ret;

	if (!args || !args[0] || args[0][0] == '\0')
	return (NULL);
	acc_ret = access_wrap(args[0], 0);
	if (acc_ret == 0)
	{
		path = ft_strdup(args[0]);
		if (!path)
			return (msh_set_error(sh, MALLOC_OP), NULL);
	}
	else if (acc_ret == -1 && errno != ENOENT)
		return (msh_set_error(sh, ACCESS_OP), NULL);
	else
	{
		acc_ret = 0;
		path = msh_path_from_cmdname(args[0], envp, sh, &acc_ret);
	}
	if (!path && acc_ret <= 0)
		return (NULL);
	return (path);
}

void stage_exit(t_shell *sh, t_cmd *cmd, int *p, int exit_code)
{
	safe_close_p(p);
	safe_close_redirs(cmd->redirs);
	free((t_stage_io *) cmd->stage_io);
	msh_print_last_error(sh);
	msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
	close(sh->save_in);
	close(sh->save_out);
	close(sh->save_err);
	free_shell_child(sh);
	free_cmd_struct(cmd);
	exit(exit_code);
}


void dup2_stage_io(t_shell *sh, t_cmd *cmd, int *p)
{
	const t_stage_io	*rdr_spec;

	rdr_spec = cmd->stage_io;
	if (rdr_spec->in_fd != -1)
		if (dup2_wrap(rdr_spec->in_fd, STDIN_FILENO) == -1)
			{
				msh_set_error(sh, DUP2_OP);
				stage_exit(sh, cmd, p, EXIT_FAILURE);
			}
	if (rdr_spec->out_fd != -1)
		if (dup2_wrap(rdr_spec->out_fd, STDOUT_FILENO) == -1)
		{
			msh_set_error(sh, DUP2_OP);
			stage_exit(sh, cmd, p, EXIT_FAILURE);
		}
}

// Entry point for execution
/*
This function is responsible for executing a command. 
Syscalls: execve, dup2, access, malloc + malloc inside libft calls ⁉️
Also this function is top level for forked process, should exit on syscall error.
Print and exits on sys error
	- dup2
	- access
	- execve
	- malloc
Before exiting: 
	- frees path if allocated
	- closes all file descriptors
	- frees redirection structures
	- frees stage_io structure
*/
void	msh_exec_stage(t_shell *sh, t_cmd *cmd, t_list *env, int *p)
{
	char				*path;
	int					st;

	dup2_stage_io(sh, cmd, p);
	path = msh_resolve_path(cmd->argv, env, sh);
	if (!path)
	{
		if (ft_strncmp(sh->last_err_op, MALLOC_OP, ft_strlen(sh->last_err_op)) == 0)
		{
			dprintf(STDERR_FILENO, "[msh_exec_stage]: '%s' memory allocation failed\n", cmd->argv[0]);
			stage_exit(sh, cmd, p, EXIT_FAILURE);
		}
		if (sh->last_errno == EACCES)
		{
			dprintf(STDERR_FILENO, "[msh_exec_stage]: '%s' permission denied\n", cmd->argv[0]);
			stage_exit(sh, cmd, p, 126);
		}
		dprintf(STDERR_FILENO, "[msh_exec_stage]: '%s' command not found\n", cmd->argv[0]);
		msh_set_error(sh, cmd->argv[0]);
		stage_exit(sh, cmd, p, 127);
	}
	if (is_builtin(cmd->argv[0]))
	{
    	st = exec_builtin(cmd, sh);
    	exit(st & 0xff);
	}
	else if (execve_wrap(path, cmd->argv, envp_from_env_list(env)) == -1)
	{
		dprintf(STDERR_FILENO, "[msh_exec_stage]: execve failed\n");
		free(path);
		st = msh_status_from_execve_error(errno);
		msh_set_error(sh, EXECVE_OP);
		stage_exit(sh, cmd, p, st);
	}
	free(path);
	exit(EXIT_FAILURE);
}

