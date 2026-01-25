#include "../include/minishell.h"

// fds_utils.c
void	safe_close_rd_fds(t_list *redirs);
int		msh_save_fds(int *save_in, int *save_out, int *save_err);
void	msh_restore_fds(int save_in, int save_out, int save_err);

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