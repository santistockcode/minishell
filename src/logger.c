/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 16:22:17 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/06 14:43:57 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include <time.h>
#include <dirent.h>

// ...existing code...
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
	t_list	*redir_node;
	t_redir	*current_redir;

	cmd_content = cmd;
	i = 0;
	if (cmd_content->redirs)
	{
		printf("\n[Redirections present");
		redir_node = cmd->redirs;
		while (redir_node)
		{
			printf(" (%d)", i);
			current_redir = (t_redir *)redir_node->content;
			printf(" type:%d", current_redir->type);
			printf(" target:%s", current_redir->target);
			redir_node = redir_node->next;
			i++;
		}
		printf("]->");

	}
	else
	{
		printf("\n[No redirections]->");
	}
	i = 0;
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
void	logger_ctx(t_shell *sh, t_list *cmd, const char *tag,
		const char *message)
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

// FIXME: don't present this (does't pass norminette)
void logger_ctx_simple(t_shell *sh, t_cmd *cmd, const char *tag, const char *message)
{
	time_t now;
	char time_str[26];

	time(&now);
	printf(COLOR_RESET);
	printf(COLOR_MAGENTA);
	if (LOG == 1)
	{
		strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
			localtime(&now));
		printf("%s [%s] (last_status: %d, last_err_op: %s): %s, COMMAND:\n",
			time_str, tag, sh->last_status,
			sh->last_err_op ? sh->last_err_op : "none", message);
		print_cmd_node(cmd);
	}
	printf(COLOR_RESET);
	printf("\n");
}

// void	logger_open_fds(const char *starttag, const char *endtag)
// {
// 	char	proc_path[64];
// 	char	cmd[128];
// 	pid_t	pid;

// 	if (LOG == 0)
// 		return ;
// 	pid = getpid();
// 	snprintf(proc_path, sizeof(proc_path), "/proc/%d/fd", pid);
// 	snprintf(cmd, sizeof(cmd), "ls -l %s 1>&2", proc_path);
// 	printf(COLOR_RESET);
// 	fprintf(stderr, COLOR_CYAN "[%s] Open file descriptors for PID %d:\n" COLOR_RESET,
// 		starttag, pid);
// 	system(cmd);
// 	fprintf(stderr, COLOR_CYAN "[%s] End of FD list\n" COLOR_RESET, endtag);
// 	printf(COLOR_RESET);
// }


// FIXME: don't present this (does't pass norminette)
void	logger_open_fds(const char *starttag, const char *endtag)
{
    char			proc_path[64];
    char			link_path[64];
    char			link_target[256];
    pid_t			pid;
    DIR				*dir;
    struct dirent	*entry;
    int				fd_num;
    ssize_t			len;
	(void)endtag;

    if (LOG == 0)
        return ;
    pid = getpid();
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/fd", pid);
    // fprintf(stderr, COLOR_CYAN "[PID %d] [%s] Open file descriptors:\n" COLOR_RESET,
    //     pid, starttag);
    dir = opendir(proc_path);
    if (!dir)
    {
        fprintf(stderr, "[PID %d] Failed to open %s\n", pid, proc_path);
        return ;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue ;
        fd_num = atoi(entry->d_name);
        if (fd_num == dirfd(dir))
            continue ;
        snprintf(link_path, sizeof(link_path), "/proc/%d/fd/%s",
            pid, entry->d_name);
        len = readlink(link_path, link_target, sizeof(link_target) - 1);
        if (len != -1)
        {
            link_target[len] = '\0';
            fprintf(stderr, "[PID %d] %s  fd %d -> %s\n", pid, starttag, fd_num, link_target);
        }
        else
            fprintf(stderr, "[PID %d] %s  fd %d -> (unknown)\n", pid, starttag, fd_num);
    }
    closedir(dir);
    // fprintf(stderr, COLOR_CYAN "[PID %d] [%s] End of FD list\n" COLOR_RESET,
    //     pid, endtag);
}


// Source - https://stackoverflow.com/a
// Posted by Edwin Buck, modified by community.
// See post 'Timeline' for change history
// Retrieved 2026-01-13, License - CC BY-SA 3.0