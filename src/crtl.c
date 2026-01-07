/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   crtl.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/04 22:08:46 by mario             #+#    #+#             */
/*   Updated: 2026/01/04 22:08:58 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

void	ft_ctrl_mini(int signal)
{
	(void)signal;
	write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
	g_exit_status = 130;
}

void	ft_ctrl_heredoc(int signal)
{
	(void)signal;
	write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	close(STDIN_FILENO);
	g_exit_status = 130;
}

void	ft_ctrl_quit(int signal)
{
	(void)signal;
	rl_on_new_line();
	rl_redisplay();
}