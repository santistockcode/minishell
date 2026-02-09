/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   crtl.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/04 22:08:46 by mario             #+#    #+#             */
/*   Updated: 2026/02/09 20:12:10 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

extern volatile sig_atomic_t	g_exit_status;

void	setup_signals_child(void)
{
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}

void	setup_signals_ignore(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}

/*
This handler is called when the user presses Ctrl+C.
It handles the signal by writing a newline to the output.
*/
void	ft_ctrl_mini(sig_atomic_t signal)
{
	(void)signal;
	write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
	g_exit_status = 130;
}

// void	ft_ctrl_heredoc(sig_atomic_t signal)
// {
// 	(void)signal;
// 	// write(1, "\n", 1);
// 	// rl_replace_line("", 0);
// 	// rl_on_new_line();
// 	// close(STDIN_FILENO);
// 	// g_exit_status = 130;

// 	// write(1, "\n", 1);
//     g_exit_status = 130;
//     rl_done = 1;
// }

// ...existing code...

// ...existing code...

void	ft_ctrl_heredoc(sig_atomic_t signal)
{
    (void)signal;
    write(1, "\n", 1);
    g_exit_status = 130;
    ioctl(STDIN_FILENO, TIOCSTI, "\n");
}

// ...existing code...

// ...existing code...

/*
This handler is called when the user presses Ctrl+\.
It handles the signal by writing a newline to the output.
*/
void	ft_ctrl_quit(sig_atomic_t signal)
{
	(void)signal;
	rl_on_new_line();
	rl_redisplay();
}
