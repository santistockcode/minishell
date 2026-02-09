/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit_utils2.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 20:51:59 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/09 21:06:51 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void	safe_close_rd_fds(t_list *redirs);
void	msh_restore_fds(int save_in, int save_out, int save_err);

/*
Custom exit because of norminette
*/
int	ebip_exit(t_shell *sh, t_list *redirs)
{
	msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
	safe_close_rd_fds(redirs);
	return (-1);
}

void	clear_saved_fds(t_shell *sh)
{
	safe_close(sh->save_in);
	safe_close(sh->save_out);
	safe_close(sh->save_err);
}
