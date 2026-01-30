#include "../include/minishell.h"

// fds_utils.c
void	safe_close_rd_fds(t_list *redirs);
int		msh_save_fds(int *save_in, int *save_out, int *save_err);
void	msh_restore_fds(int save_in, int save_out, int save_err);

/* EXEC_PIPELINE */

/*
Not checkign if nstages < 1 because of norminette.
Do not call run_pipeline with incorrect cmd_first.
Returns -1 on fork error (ya sea en first, middle or last)
*/
int run_pipeline(t_shell *sh, t_list *cmd_first, int nstages, pid_t *pid)
{
	int p[2];
	int in_fd;
	t_list *current_cmd_node;

	sh->cmds_start = cmd_first;
	if (pipe_wrap(p) == -1)
		return (msh_set_error(sh, PIPE_OP), -1);
	current_cmd_node = cmd_first;
	if (do_first_command(sh, (t_cmd *)current_cmd_node->content, p) == -1)
		return (-1);
	in_fd = p[0];
	current_cmd_node = current_cmd_node->next;
	while (nstages-- > 2)
	{
		if (pipe_wrap(p) == -1)
			return (msh_set_error(sh, PIPE_OP), -1); 
		if (do_middle_commands(sh, (t_cmd *)current_cmd_node->content, p, in_fd) == -1)
			return (-1);
		in_fd = p[0];
		current_cmd_node = current_cmd_node->next;
	}
	sh->cmds_start = NULL;
	return (do_last_command(sh, (t_cmd *)current_cmd_node->content, in_fd, pid));
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

// void detailed_logger(t_list *cmd_first)
// {
// 	t_list *current = cmd_first;
// 	int i;
// 	i = 0;
// 	while (current)
// 	{
// 		t_cmd *cmd = (t_cmd *)current->content;
// 		t_list *redirs = cmd->redirs;
// 		printf("Command %d:", i);
// 		if (cmd->argv)
// 		{
// 			int j = 0;
// 			while (cmd->argv[j])
// 			{
// 				printf(" argv[%d]=%s", j, cmd->argv[j]);
// 				j++;
// 			}
// 		}
// 		else
// 		{
// 			printf(" [No arguments in argv]");
// 		}
// 		while (redirs)
// 		{
// 			t_redir *redir = (t_redir *)redirs->content;
// 			printf("Redirection: type=%d, fd=%d, target=%s, quoted=%d\n",
// 				redir->type,
// 				redir->fd,
// 				redir->target,
// 				redir->quoted);
// 			redirs = redirs->next;
// 		}
// 		i++;
// 		current = current->next;
// 	}
// }

// returns -1 on fork error happened before last command on parent
// it interrupted pipeline: FATAL!
int msh_exec_pipeline(t_shell *sh, t_list *cmd_first, int nstages)
{
	logger_ctx(sh, cmd_first, "EXEC_PIPELINE", "[line 93]");
	//detailed_logger(cmd_first);
	if (require_standard_fds(sh) == -1)
		return (-1);
	int result;
	int wtpd_resp;
	int status;
	pid_t pid;

	result = run_pipeline(sh, cmd_first, nstages, &pid);
	if (result == -1)
		return (-1); 
	wtpd_resp = waitpid(pid, &status, 0);
	if (wtpd_resp == -1)
		return (msh_set_error(sh, WAITPID_OP), -1);
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	return (result);
}
