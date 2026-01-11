/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/12 14:15:00 by mario             #+#    #+#             */
/*   Updated: 2026/01/11 13:22:17 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

extern volatile sig_atomic_t	exit_status;

void septup_signal(void)
{
	struct sigaction	sa;
	
	ft_memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = ft_ctrl_mini;
	sigaction(SIGINT, &sa, NULL);
	sa.sa_handler = ft_ctrl_quit;
	sigaction(SIGQUIT, &sa, NULL);
}
void	setup_signals_heredoc(void)
{
	struct sigaction	sa;

	ft_memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = ft_ctrl_heredoc;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}
