/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/12 14:15:00 by mario             #+#    #+#             */
/*   Updated: 2025/12/12 14:38:45 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

extern int	g_exit_status;

void septup_signal(void )
{
	struct sigaction	sa;
	ft_memset(&sa, 0, sizeof(sa));
	sa.sa_flag = SA_RESTART;
	//sa.sa_handler = ;
	sigaction(SIGINT, &sa, NULL);
	//sa.sa_handler = ;
	sigaction(SIGQUIT, &sa, NULL);

}