/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 16:22:17 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/13 16:44:45 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include <time.h>

// FIXME: do not present this file with time.h
// void    valid_logger(const char* tag, const char* message)
// {
//     (void)tag;
//     (void)message;
// }

// void valid_logger_ctx(const char *tag, const char *message,
// t_shell *sh, t_list *cmd)
// {
//     (void)tag;
//     (void)message;
//     (void)sh;
//     (void)cmd;
// }

void	print_cmd_node(t_cmd *cmd)
{
	int		i;
	t_cmd	*cmd_content;

	cmd_content = cmd;
	i = 0;
	if (cmd_content->redirs)
	{
		printf("[Redirections present]->");
	}
	else
	{
		printf("[No redirections]->");
	}
	if (cmd_content->argv)
	{
		while (cmd_content->argv[i])
		{
			printf("argv[%d]: %s   ", i, cmd_content->argv[i]);
			i++;
		}
	}
	else
	{
		printf("    [No arguments in argv]\n");
	}
}

void	logger(const char *tag, const char *message)
{
	time_t	now;
	char	time_str[26];

	printf(COLOR_RESET);
	printf(COLOR_YELLOW);
	time(&now);
	if (LOG == 1)
	{
		strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
			localtime(&now));
		printf("%s [%s]: %s\n", time_str, tag, message);
	}
	printf(COLOR_RESET);
}

// FIXME: don't present this (does't pass norminette)
void	logger_ctx(t_shell *sh, t_list *cmd,
	const char *tag, const char *message)
{
	time_t	now;
	char	time_str[26];

	time(&now);
	printf(COLOR_RESET);
	printf(COLOR_MAGENTA);
	if (LOG == 1)
	{
		strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
			localtime(&now));
		printf("%s [%s] (last_status: %d, last_err_op: %s): %s, COMMANDS:\n",
			time_str, tag, sh->last_status,
			sh->last_err_op ? sh->last_err_op : "none", message);
		ft_lstiter(cmd, (void (*)(void *))print_cmd_node);
	}
	printf(COLOR_RESET);
	printf("\n");
}

// Source - https://stackoverflow.com/a
// Posted by Edwin Buck, modified by community.
// See post 'Timeline' for change history
// Retrieved 2026-01-13, License - CC BY-SA 3.0