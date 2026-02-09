/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/12 14:15:00 by mario             #+#    #+#             */
/*   Updated: 2026/02/09 20:17:10 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

extern volatile sig_atomic_t	g_exit_status;

// static int	hd_event_hook(void)
// {
//     if (g_exit_status == 130)
//     {
//         rl_done = 1;
//         return (1);
//     }
//     return (0);
// }

/*
This function sets up the signal handlers for the shell.
*/
void	setup_signal(void)
{
	struct sigaction	sa;

	ft_memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = ft_ctrl_mini;
	sigaction(SIGINT, &sa, NULL);
	sa.sa_handler = ft_ctrl_quit;
	sigaction(SIGQUIT, &sa, NULL);
}

/*
This function sets up the signal handlers for the heredoc.
*/
// ...existing code...

// ...existing code...

void	setup_signals_heredoc(void)
{
    struct sigaction	sa;

    ft_memset(&sa, 0, sizeof(sa));
    sa.sa_flags = 0;
    sa.sa_handler = ft_ctrl_heredoc;
    sigaction(SIGINT, &sa, NULL);
    signal(SIGQUIT, SIG_IGN);
}

// ...existing code...

// ...existing code...
// void	setup_signals_heredoc(void)
// {
//     struct sigaction	sa;

//     ft_memset(&sa, 0, sizeof(sa));
//     sa.sa_flags = 0;
//     sa.sa_handler = ft_ctrl_heredoc;
//     sigaction(SIGINT, &sa, NULL);
//     signal(SIGQUIT, SIG_IGN);
// 	rl_event_hook = hd_event_hook;
// }
