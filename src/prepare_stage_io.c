/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prepare_stage_io.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 18:06:54 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/06 20:28:50 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

int			get_r_in_redir_fd(t_list *redirs);
int			get_r_out_redir_fd(t_list *redirs);
int			get_r_out_mode(t_list *redirs);


void	safe_close_p(int *p);

/*
Here's the thing, what should actually happen on first/middle/last cmd
when we have both redirs and pipes, specifically in bash: 
- on first cmd: out redir have priority over pipe
- on middle cmd: in redir have priority over pipe, out redir have priority over pipe
- on last cmd: in redir have priority over pipe

In case of multiple redirs, last one wins.

All unused file descriptors should be closed.
*/


/*
If there's an input redirection on last command, use it;
otherwise use pipe.
If there's an output redirection in last cmd, use it. 
*/
void	assign_last(t_stage_io **rdr_spec, t_list *redirs, int in_fd)
{
	int	in_redir_fd;
	int	out_redir_fd;

	in_redir_fd = get_r_in_redir_fd(redirs);
	out_redir_fd = get_r_out_redir_fd(redirs);
	if (in_redir_fd != -1 && in_redir_fd != in_fd)
	{
		(*rdr_spec)->in_fd = in_redir_fd;
		safe_close(in_fd);
	}
	else
		(*rdr_spec)->in_fd = in_fd;
	(*rdr_spec)->out_fd = out_redir_fd;
	(*rdr_spec)->out_mode = get_r_out_mode(redirs);
}

/*
If theres an input redirect in middle cmd, use it
Otherwise take input from pipe.
If ther's an ouput redirection in middle cmd, use it.
Otherwise, pipe it.
*/
void	assign_middle(t_stage_io **rdr_spec, t_list *redirs, int *p, int in_fd) // p has just been created, in_fd comes from stage 1;
{
	int	in_redir_fd;
	int	out_redir_fd;

	in_redir_fd = get_r_in_redir_fd(redirs);
	out_redir_fd = get_r_out_redir_fd(redirs);
	if (in_redir_fd != -1 && in_redir_fd != in_fd)
	{
		(*rdr_spec)->in_fd = in_redir_fd;
		safe_close(in_fd);
	}
	else
	{
		(*rdr_spec)->in_fd = in_fd;
	}
	if (out_redir_fd != -1)
	{
		(*rdr_spec)->out_fd = out_redir_fd;
		(*rdr_spec)->out_mode = get_r_out_mode(redirs);
		safe_close(p[1]); 
		safe_close(p[0]);
	}
	else
	{
		(*rdr_spec)->out_fd = p[1];
		(*rdr_spec)->out_mode = 0;
		safe_close(p[0]);
	}
}

/*
If there is an input redirection, use it; otherwise, default -1
If there is an output redirection, use it; otherwise, pipe it
*/
void	assign_first(t_stage_io **rdr_spec, t_list *redirs, int *p)
{
	int	out_redir_fd;

	(*rdr_spec)->in_fd = get_r_in_redir_fd(redirs);
	out_redir_fd = get_r_out_redir_fd(redirs);
	if (out_redir_fd != -1)
	{
		(*rdr_spec)->out_fd = out_redir_fd;
		(*rdr_spec)->out_mode = get_r_out_mode(redirs);
		safe_close(p[1]);
		safe_close(p[0]);
	}
	else
	{
		(*rdr_spec)->out_fd = p[1];
		(*rdr_spec)->out_mode = 0;
	}
}

/*
 * Prepare the stage IO structure based on the position and redirections.
 */
t_stage_io	*prepare_stage_io(t_stage_type pos, t_list *redirs, int in_fd,
		int *p)
{
	t_stage_io	*rdr_spec;

	rdr_spec = malloc(sizeof(t_stage_io));
	if (!rdr_spec)
		return (NULL);
	if (pos == FIRST)
		assign_first(&rdr_spec, redirs, p);
	else if (pos == MIDDLE)
		assign_middle(&rdr_spec, redirs, p, in_fd);
	else if (pos == LAST)
		assign_last(&rdr_spec, redirs, in_fd);
	return (rdr_spec);
}
