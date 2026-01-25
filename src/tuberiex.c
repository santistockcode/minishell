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

// fds_utils.c
void	safe_close_rd_fds(t_list *redirs);
int		msh_save_fds(int *save_in, int *save_out, int *save_err);
void	msh_restore_fds(int save_in, int save_out, int save_err);


/* REDIRS STAGE IO HELPERS*/

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

/* PREPARE REDIRS */

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

/* PREPARE STAGE IO */

t_stage_io  *prepare_stage_io(t_stage_type pos, t_list *redirs, int in_fd, int *p)
{
	t_stage_io *rdr_spec;
	int out_redir_fd;
	int in_redir_fd;

	rdr_spec = malloc(sizeof(t_stage_io));
	if (!rdr_spec)
		return (NULL);
	out_redir_fd = get_r_out_redir_fd(redirs);
	if (pos == FIRST)
	{
		rdr_spec->in_fd = get_r_in_redir_fd(redirs);
        if (out_redir_fd != -1)
        {
            rdr_spec->out_fd = out_redir_fd;
            rdr_spec->out_mode = get_r_out_mode(redirs);
        }
        else
        {
            rdr_spec->out_fd = p[1];
            rdr_spec->out_mode = 0;
        }
	}
	else if (pos == MIDDLE)
	{
		in_redir_fd = get_r_in_redir_fd(redirs);
		if (in_redir_fd != -1 && in_redir_fd != in_fd)
		{
			rdr_spec->in_fd = in_redir_fd;
			safe_close(in_fd);
		}
		else
			rdr_spec->in_fd = in_fd;
        if (out_redir_fd != -1)
        {
            rdr_spec->out_fd = out_redir_fd;
            rdr_spec->out_mode = get_r_out_mode(redirs);
        }
        else
        {
            rdr_spec->out_fd = p[1];
            rdr_spec->out_mode = 0;
        }
	}
	else if (pos == LAST)
	{
		in_redir_fd = get_r_in_redir_fd(redirs);
		if (in_redir_fd != -1 && in_redir_fd != in_fd)
		{
			rdr_spec->in_fd = in_redir_fd;
			safe_close(in_fd);
		}
		else
			rdr_spec->in_fd = in_fd;
		rdr_spec->out_fd = out_redir_fd;
		rdr_spec->out_mode = get_r_out_mode(redirs);
	}
	return (rdr_spec);
}

/* DO FIRST */

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
		msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
		redirs = cmd->redirs;
		if (prepare_redirs(redirs) == -1)
			return (safe_close_rd_fds(redirs), -1); // fixme: should NOT exit here
		cmd->stage_io = prepare_stage_io(LAST, redirs, last_fd, p);
		if (!cmd->stage_io)
			return (safe_close_rd_fds(redirs), -1); // fixme: should exit here
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

/* DO MIDDLE */

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
		msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
		redirs = cmd->redirs;
		if (prepare_redirs(redirs) == -1)
			return (safe_close_rd_fds(redirs), -1); // FIXME: should exit here
		rdr_spec = prepare_stage_io(MIDDLE, redirs, in_fd, p);
		if (!rdr_spec)
			return (safe_close_rd_fds(redirs), -1); // FIXME: should exit here
		cmd->stage_io = rdr_spec;
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(in_fd);
	safe_close(p[1]);
	return (0);
}

/* DO LAST */

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
			return (safe_close_rd_fds(redirs), -1); // fixme: should exit here
		rdr_spec = prepare_stage_io(FIRST, redirs, -1, p);
		if (!rdr_spec)
			return (safe_close_rd_fds(redirs), -1); // fixme: should exit here
		cmd->stage_io = rdr_spec;
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(p[1]);
	return (0);
}

/* EXEC_PIPELINE */

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
