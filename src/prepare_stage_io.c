#include "../include/minishell.h"

int get_r_in_redir_fd(t_list *redirs);
int get_r_out_redir_fd(t_list *redirs);
int get_r_out_mode(t_list *redirs);


// TODO: refactor to norminette (shouldn't be difficult)
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
