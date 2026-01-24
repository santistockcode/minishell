#include "../include/minishell.h"

/*
	*************************************************
	*                                               *
	*PRUEBA DE CONCEPTO PIPEX INTEGRADO EN MINISHELL*
	*                                               *
	*       /\_/|                                   *
	*      ( o.o )  y si no lo acabamos??!          *
	*       > ^ <                                   *
	*                                               *
	*************************************************
*/


/* EXEC CMD IN PIPELINE*/

void    safe_close_redirs(t_list *redirs)
{
	t_redir *redir;

	while (redirs)
	{
		redir = (t_redir *)redirs->content;
		if (redir)
		{
			if (safe_close(redir->fd) == -1)
			{
				msh_set_error(NULL, CLOSE_OP);
			}
		}
		redirs = redirs->next;
	}
}

// const char *get_path_envp(t_list *env)
// {
//     t_env *env_var;

//     dprintf(STDERR_FILENO, "[get_path_envp]: Searching for PATH\n");
//     while (env)
//     {
//         env_var = (t_env *)env->content;
//         if (env_var && ft_strncmp(env_var->key, "PATH", 4) == 0)
//             return (env_var->value);
//         env = env->next;
//     }
//     return (NULL);
// }

// void	ft_split_free(char **paths)
// {
// 	char	**tmp;

// 	if (!paths)
// 		return ;
// 	tmp = paths;
// 	while (*tmp)
// 	{
// 		free(*tmp);
// 		tmp++;
// 	}
// 	free(paths);
// }

// char	*msh_path_from_cmdname(char *arg, t_list *env, t_shell *sh)
// {
// 	char	**paths;
// 	char	*path;
// 	char	*bar;
// 	char	**paths_start;
//     const char *env_value;

// 	env_value = get_path_envp(env);
// 	if (!env_value || !(*env_value))
// 		return (NULL);
// 	paths = ft_split(env_value, ':');
// 	paths_start = paths;
// 	while (paths && *paths)
// 	{
// 		bar = ft_strjoin(*paths, "/");
// 		path = ft_strjoin(bar, arg);
// 		free(bar);
// 		if (access(path, F_OK) == 0)
// 			return (msh_set_error(sh, ACCESS_OP), ft_split_free(paths_start), path);
// 		free(path);
// 		paths++;
// 	}
// 	ft_split_free(paths_start);
// 	return (NULL);
// }

// char *msh_resolve_path(char **args, t_list *envp, t_shell *sh)
// {
//     char *path;

//     if (!args || !args[0] || args[0][0] == '\0')
//         return NULL;
//     if (access(args[0], 0) == 0)
//         path = ft_strdup(args[0]);
//     else
//         path = msh_path_from_cmdname(args[0], envp, sh);
//     dprintf(STDERR_FILENO, "[msh_resolve_path]: %s\n", path);
//     return path;
// }

// char *const *envp_from_env_list(t_list *env)
// {
//     t_env *env_var;
//     char **envp;
//     char *pair;
//     int size;
//     int i;

//     size = ft_lstsize(env);
//     envp = (char **)malloc(sizeof(char *) * (size + 1));
//     if (!envp)
//         return (NULL);
//     i = 0;
//     while (env)
//     {
//         env_var = (t_env *)env->content;
//         if (env_var)
//         {
//             pair = ft_strjoin(env_var->key, "=");
//             envp[i] = ft_strjoin(pair, env_var->value);
//             free(pair);
//             i++;
//         }
//         env = env->next;
//     }
//     envp[i] = NULL;
//     return (envp);
// }

// int msh_exec_stage(t_shell *sh, t_cmd *cmd,
//     const t_stage_io *io, t_list *env, int *p)
// {
//     char *path;
//     int st;

//     logger("msh_exec_stage", "Preparing stage");
//         /* only dup2 if the fd is valid */
//     if (io->in_fd != -1)
//     {
//         if (dup2_wrap(io->in_fd, STDIN_FILENO) == -1)
//         {
//             logger("msh_exec_stage", "dup2 failed (in)");
//             if (p)
//             {
//                 safe_close(p[0]);
//                 safe_close(p[1]);
//             }
//             return -1;
//         }
//     }
//     if (io->out_fd != -1)
//     {
//         if (dup2_wrap(io->out_fd, STDOUT_FILENO) == -1)
//         {
//             logger("msh_exec_stage", "dup2 failed (out)");
//             if (p)
//             {
//                 safe_close(p[0]);
//                 safe_close(p[1]);
//             }
//             return -1;
//         }
//     }
//     if (p)
//     {
//         safe_close(p[0]);
//         safe_close(p[1]);
//     }
//     dprintf(STDERR_FILENO, "[msh_exec_stage]: Preparing stage\n");
//     path = msh_resolve_path(cmd->argv, env, sh);
//     if (!path)
//     {
//         dprintf(STDERR_FILENO, "[msh_exec_stage]: Command not found\n");
//         return (exit_status = 127, -1); // pending testing
//     }
//     dprintf(STDERR_FILENO, "[msh_exec_stage:path]: %s\n", path);
//     if (execve_wrap(path, cmd->argv, envp_from_env_list(env)) == -1)
//     {
//         dprintf(STDERR_FILENO, "[msh_exec_stage]: execve failed\n");
//         st = msh_status_from_execve_error(errno);
//         msh_set_error(sh, EXECVE_OP);
//         msh_print_last_error(sh); 
//         exit(st);
//     }
//     dprintf(STDERR_FILENO, "[msh_exec_stage]: Execution failed\n");
//     free(path);
//     exit(EXIT_FAILURE);
// }

/* EXEC CMD IN PIPELINE UTILS */

int get_r_in_redir_fd(t_list *redirs)
{
	t_redir *redir;

	while (redirs)
	{
		redir = (t_redir *)redirs->content;
		if (redir && redir->type == R_IN)
			return (redir->fd);
		redirs = redirs->next;
	}
	return (-1);
}

// it fetches the last output redir (example: "cat < infile > outfile1 > outfile2" should write to outfile2)
int get_r_out_redir_fd(t_list *redirs)
{
	t_redir *redir;

	while (redirs)
	{
		redir = (t_redir *)redirs->content;
		if (redir && (redir->type == R_OUT_TRUNC || redir->type == R_OUT_APPEND) && redirs->next == NULL)
			return (redir->fd);
		redirs = redirs->next;
	}
	return (-1);
}

int get_r_out_mode(t_list *redirs)
{
	t_redir *redir;

	while (redirs)
	{
		redir = (t_redir *)redirs->content;
		if (redir && (redir->type == R_OUT_TRUNC || redir->type == R_OUT_APPEND) && redirs->next == NULL) 
		{
			if (redir->type == R_OUT_TRUNC)
				return (OM_TRUNC);
			else if (redir->type == R_OUT_APPEND)
				return (OM_APPEND);
		}
		redirs = redirs->next;
	}
	return (-1);
}

int prepare_redirs(t_list *redirs)
{
    t_redir *redir;

    while (redirs)
    {
        redir = (t_redir *)redirs->content;
        logger("prepare_redirs:REDIR-TARGET", redir->target);
        if (redir && redir->type == R_IN)
        {
            logger("prepare_redirs", "Opening input redirection");
            redir->fd = open_wrap(redir->target, O_RDONLY, 0);
            if (redir->fd == -1)
                return (msh_set_error(NULL, OPEN_OP), -1);
        }
        else if(redir && redir->type == R_OUT_APPEND)
        {
            logger("prepare_redirs", "Opening output append redirection");
            redir->fd = open_wrap(redir->target, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (redir->fd == -1)
                return (msh_set_error(NULL, OPEN_OP), -1);
        }
        else if (redir && redir->type == R_OUT_TRUNC)
        {
            logger("prepare_redirs", "Opening output truncation redirection");
            redir->fd = open_wrap(redir->target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (redir->fd == -1)
                return (msh_set_error(NULL, OPEN_OP), -1);
        }
        redirs = redirs->next;
    }
    return (0);
}

int msh_save_fds(int *save_in, int *save_out, int *save_err)
{
    *save_in = dup(STDIN_FILENO);
    *save_out = dup(STDOUT_FILENO);
    *save_err = dup(STDERR_FILENO);
    if (*save_in == -1 || *save_out == -1 || *save_err == -1)
    {
        if (*save_in != -1) close(*save_in);
        if (*save_out != -1) close(*save_out);
        if (*save_err != -1) close(*save_err);
        return (-1);
    }
    return (0);
}

void msh_restore_fds(int save_in, int save_out, int save_err)
{
    if (save_in != -1)
    {
        dup2(save_in, STDIN_FILENO);
        close(save_in);
    }
    if (save_out != -1)
    {
        dup2(save_out, STDOUT_FILENO);
        close(save_out);
    }
    if (save_err != -1)
    {
        dup2(save_err, STDERR_FILENO);
        close(save_err);
    }
}

// FIXME: what should happen in echo "example test" > outfile | sleep 3
// The output redirection should be set up correctly for the pipeline

t_stage_io  *prepare_stage_io(t_stage_type pos, t_list *redirs, int in_fd, int *p)
{
	t_stage_io *rdr_spec;

	rdr_spec = malloc(sizeof(t_stage_io));
	if (!rdr_spec)
		return (NULL);
	if (pos == FIRST)
	{
		rdr_spec->in_fd = get_r_in_redir_fd(redirs);
		rdr_spec->out_fd = p[1];
		rdr_spec->out_mode = 0;
	}
	else if (pos == MIDDLE)
	{
		rdr_spec->in_fd = in_fd;
		rdr_spec->out_fd = p[1];
		rdr_spec->out_mode = 0;
	}
	else if (pos == LAST)
	{
		rdr_spec->in_fd = in_fd;
		rdr_spec->out_fd = get_r_out_redir_fd(redirs);
		rdr_spec->out_mode = get_r_out_mode(redirs);
	}
	return (rdr_spec);
}

int do_last_command(t_shell *sh, t_cmd *cmd, int last_fd)
{
	pid_t pid;
	t_list *redirs;
	int status;
	int *p;
	int wtpd_resp;

	pid = fork_wrap(); // from here, better to just exit
	p = NULL;
	if (pid < 0)
		return (msh_set_error(sh, FORK_OP), -1);
	if (pid == 0)
	{
		//FIXME: need to save og fds?
		redirs = cmd->redirs;
		if (prepare_redirs(redirs) == -1)
			return (safe_close_redirs(redirs), -1); // fixme: should NOT exit here
		cmd->stage_io = prepare_stage_io(LAST, redirs, last_fd, p);
		if (!cmd->stage_io)
			return (safe_close_redirs(redirs), -1); // fixme: should exit here
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(last_fd);
	wtpd_resp = waitpid(pid, &status, 0);
	if (wtpd_resp == -1)
		return (msh_set_error(sh, WAITPID_OP), -1);
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	return (-1); // unknown error
}

// FIXME: turns out you can have a > redir in the middle of a pipeline (fuck)
int do_middle_commands(t_shell *sh, t_cmd *cmd, int *p, int in_fd)
{
	pid_t pid;
	// int fd;
	t_list *redirs;
	t_stage_io *rdr_spec;


	pid = fork_wrap();
	if (pid < 0)
		return (msh_set_error(sh, FORK_OP), -1);
	if (pid == 0)
	{
		//FIXME: need to save og fds?
		redirs = cmd->redirs;
		rdr_spec = prepare_stage_io(MIDDLE, redirs, in_fd, p);
		if (!rdr_spec)
			return (safe_close_redirs(redirs), -1); // fixme: should exit here
		cmd->stage_io = rdr_spec;
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(in_fd);
	safe_close(p[1]);
	return (0);
}

int do_first_command(t_shell *sh, t_cmd *cmd, int *p)
{
	pid_t pid;
	t_list *redirs;
	t_stage_io *rdr_spec;

	pid = fork_wrap();
	if (pid < 0)
		return (msh_set_error(sh, FORK_OP), -1);
	if (pid == 0) // child process, should exit
	{
		msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
		redirs = cmd->redirs;
		if (prepare_redirs(redirs) == -1)
			return (safe_close_redirs(redirs), -1); // fixme: should exit here
		rdr_spec = prepare_stage_io(FIRST, redirs, -1, p);
		if (!rdr_spec)
			return (safe_close_redirs(redirs), -1); // fixme: should exit here
		cmd->stage_io = rdr_spec;
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(p[1]);
	return (0);
}


/*
Not checkign if nstages < 1 because of norminette.
Do not call run_pipeline with incorrect cmd_first.
Returns (-1) on error (because that's what do_last_command returns)
*/
int run_pipeline(t_shell *sh, t_list *cmd_first, int nstages)
{
	int p[2];
	int i;
	int in_fd;
	t_list *current_cmd_node;

	if (pipe_wrap(p) == -1)
		return (msh_set_error(sh, PIPE_OP), -1);
	current_cmd_node = cmd_first;
	if (do_first_command(sh, (t_cmd *)current_cmd_node->content, p) == -1)
		return (-1);
	in_fd = p[0];
	i = 1;
	current_cmd_node = current_cmd_node->next;
	while (i < nstages - 1)
	{
		if (pipe_wrap(p) == -1)
			return (msh_set_error(sh, PIPE_OP), -1);
		if (do_middle_commands(sh, (t_cmd *)current_cmd_node->content, p, in_fd) == -1)
			return (-1);
		in_fd = p[0];
		current_cmd_node = current_cmd_node->next;
		i++;
	}
	return (do_last_command(sh, (t_cmd *)current_cmd_node->content, in_fd));
}

int require_standard_fds(t_shell *sh)
{
	struct stat *statbuf;
	int fd;

	fd = 0;
	statbuf = malloc(sizeof(struct stat));
	if (!statbuf)
		return (msh_set_error(sh, MALLOC_OP), -1);
	while (fd <= 2)
	{
		if (fstat(fd, statbuf) == -1)
		{
			msh_set_error(sh, MISSING_FDS_OP);
			return (-1);
		}
		fd++;
	}
	free(statbuf);
	return (0);
}

void detailed_logger(t_list *cmd_first)
{
	t_list *current = cmd_first;
	int i;
	i = 0;
	while (current)
	{
		t_cmd *cmd = (t_cmd *)current->content;
		t_list *redirs = cmd->redirs;
		printf("Command %d\n", i);
		while (redirs)
		{
			t_redir *redir = (t_redir *)redirs->content;
			printf("Redirection: type=%d, fd=%d, target=%s, quoted=%d\n",
				redir->type,
				redir->fd,
				redir->target,
				redir->quoted);
			redirs = redirs->next;
		}
		i++;
		current = current->next;
	}
}


int msh_exec_pipeline(t_shell *sh, t_list *cmd_first, int nstages)
{
	detailed_logger(cmd_first);
	if (require_standard_fds(sh) == -1)
		return (-1);
	return (run_pipeline(sh, cmd_first, nstages));
}

int msh_exec_simple(t_shell __attribute__((unused)) *sh, t_cmd __attribute__((unused)) *cmd, t_list __attribute__((unused)) *env)
{
	logger("msh_exec_simple", "Not implemented yet");
	return (0);
}
