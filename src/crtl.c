/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   crtl.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/04 22:08:46 by mario             #+#    #+#             */
/*   Updated: 2026/01/12 12:41:34 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void	setup_signals_child(void)
{
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_IGN);
}

void	setup_signals_ignore(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}
void	ft_ctrl_mini(sig_atomic_t signal)
{
	(void)signal;
	write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
	exit_status = 130;
}

void	ft_ctrl_heredoc(sig_atomic_t signal)
{
	(void)signal;
	write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	close(STDIN_FILENO);
	exit_status = 130;
}

void	ft_ctrl_quit(sig_atomic_t signal)
{
	(void)signal;
	rl_on_new_line();
	rl_redisplay();
}