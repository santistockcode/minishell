/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_stage.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 18:17:59 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/05 12:01:30 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

// exec_utils
void		exit_from_no_path(t_shell *sh, t_cmd *cmd, int *p);

// path utils
void		ft_spfr(char **paths);
const char	*get_path_envp(t_list *env);
char		*ft_strjoin_prot(char const *str1, char const *str2);
char	*const	*envp_from_env_list(t_list *env);
char		*build_path(const char *dir, const char *file);

// fds utils
void		safe_close_rd_fds(t_list *redirs);
void		dup2_stage_io(t_shell *sh, t_cmd *cmd, int *p);
int			msh_save_fds(int *save_in, int *save_out, int *save_err);
void		msh_restore_fds(int save_in, int save_out, int save_err);

// exit utils
void		free_shell_child(t_shell *sh);
void		free_cmd_struct(void *input);
void		stage_exit_print(t_shell *sh, t_cmd *cmd, int *p, int exit_code);

// free cmds
void		free_envp(const char **envp);

// builtins_orq
void		builtin_stage_exit(t_shell *sh, t_cmd *cmd, int *p, int exit_code);

/*
Returns:
- path is the path allocated if found, NULL if not found
- acc_ret is set to 0 on access didn't fail, -1 on syscall error
(access failed ENOENT excluded)
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
int	try_access(char **pre_paths, char **path, char *file, int *acc_ret)
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
acc_ret should be set to 0 on success or malloc fail,
-1 on error syscall if not ENOENT.
*/
char	*msh_path_from_cmdname(char *arg, t_list *env, t_shell *sh,
		int *acc_ret)
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
			return (ft_spfr(paths_start), path);
		else if (*acc_ret == (-2))
			return (msh_set_error(sh, MALLOC_OP), ft_spfr(paths_start), (NULL));
		else if (*acc_ret == (-1))
			return (msh_set_error(sh, ACCESS_OP), ft_spfr(paths_start), (NULL));
		all_paths++;
	}
	return (ft_spfr(paths_start), *acc_ret = 0, (NULL));
}

// returns path on found or NULL on not found and errors (sh->errno already set)
/*
This function resolves the path of a command by checking
if it's directly accessible
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

void	msh_exec_builtin_child(t_shell *sh, t_cmd *cmd, int *p)
{
	int	st;

	st = exec_builtin(cmd, sh);
	builtin_stage_exit(sh, cmd, p, st);
}

// Entry point for execution
/*
This function is responsible for executing a command.
Syscalls: execve, dup2, access, malloc + malloc inside libft calls ⁉️
Also this function is top level for forked process,
	should exit on syscall error.
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
	char		*path;
	int			st;
	char *const	*envp;

	dup2_stage_io(sh, cmd, p);
	if (is_builtin(cmd->argv[0]))
		msh_exec_builtin_child(sh, cmd, p);
	path = msh_resolve_path(cmd->argv, env, sh);
	if (!path)
		exit_from_no_path(sh, cmd, p);
	envp = envp_from_env_list(env);
	if (!envp)
	{
		free(path);
		stage_exit_print(sh, cmd, p, EXIT_FAILURE);
	}
	safe_close(sh->save_in);
	safe_close(sh->save_out);
	safe_close(sh->save_err);
	if (execve_wrap(path, cmd->argv, envp) == -1)
	{
		free(path);
		free_envp((const char **)envp);
		st = msh_status_from_execve_error(errno);
		msh_set_error(sh, EXECVE_OP);
		stage_exit_print(sh, cmd, p, st);
	}
	logger_open_fds( "🔥[msh_exec_stage.c]fuckedup🔥", "[msh_exec_stage.c]fuckedup");
	exit(EXIT_FAILURE);
}
/*
lr-x------ 1 saalarco 2019 64 Jan 29 17:37 0 ->
	/home/saalarco/Dev/minishell/tests/unit/mock-files/infile.txt
l-wx------ 1 saalarco 2019 64 Jan 29 17:37 1 -> 'pipe:[495373]'
lrwx------ 1 saalarco 2019 64 Jan 29 17:37 2 -> /dev/pts/5
lr-x------ 1 saalarco 2019 64 Jan 29 17:37 3 -> 'pipe:[495373]'
lrwx------ 1 saalarco 2019 64 Jan 29 17:37 5 -> /dev/pts/1
lrwx------ 1 saalarco 2019 64 Jan 29 17:37 6 -> /dev/pts/4
lrwx------ 1 saalarco 2019 64 Jan 29 17:37 7 -> /dev/pts/5
*/
