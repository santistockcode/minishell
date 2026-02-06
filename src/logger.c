/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 16:22:17 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/06 20:28:50 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
#include <time.h>
#include <dirent.h>

static const char	*redir_exec_type_to_str(t_redir_type type);

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

void	logger_ctx_simple(t_shell *sh, t_cmd *cmd, const char *tag,
		const char *message)
{
	time_t	now;
	char	time_str[26];
	t_list	*redir_node;
	t_redir	*redir;
	int		idx;

	if (LOG != 1)
		return ;
	time(&now);
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
		localtime(&now));
	printf(COLOR_RESET);
	printf(COLOR_CYAN);
	printf("%s [%s] (last_status: %d, last_err_op: %s): %s\n",
		time_str, tag ? tag : "(no tag)",
		sh ? sh->last_status : -1,
		(sh && sh->last_err_op) ? sh->last_err_op : "none",
		message ? message : "(no message)");
	if (!cmd)
	{
		printf("  cmd=(null)\n");
		printf(COLOR_RESET);
		return ;
	}
	if (!cmd->argv)
		printf("  argv=(null)\n");
	else
	{
		idx = 0;
		while (cmd->argv[idx])
		{
			printf("  argv[%d]=\"%s\"\n", idx, cmd->argv[idx]);
			idx++;
		}
	}
	redir_node = cmd->redirs;
	if (!redir_node)
		printf("  redirs=(null)\n");
	while (redir_node)
	{
		redir = (t_redir *)redir_node->content;
		if (redir)
		{
			printf("  redir type=%s fd=%d target=\"%s\" quoted=%d\n",
				redir_exec_type_to_str(redir->type), redir->fd,
				redir->target ? redir->target : "(null)",
				redir->quoted);
		}
		redir_node = redir_node->next;
	}
	printf(COLOR_RESET);
}

static const char	*token_type_to_str(t_token_type type)
{
	if (type == TOKEN_IGNORE)
		return ("IGNORE");
	if (type == TOKEN_PIPE)
		return ("PIPE");
	if (type == TOKEN_WORD)
		return ("WORD");
	if (type == TOKEN_REDIR_IN)
		return ("REDIR_IN");
	if (type == TOKEN_REDIR_OUT)
		return ("REDIR_OUT");
	if (type == TOKEN_REDIR_APPEND)
		return ("REDIR_APPEND");
	if (type == TOKEN_HEREDOC)
		return ("HEREDOC");
	if (type == TOKEN_ASSIGN_WORD)
		return ("ASSIGN_WORD");
	if (type == TOKEN_EOF)
		return ("EOF");
	return ("UNKNOWN");
}

static const char	*redir_type_to_str(t_io_redirect_type type)
{
	if (type == REDIR_IN)
		return ("<");
	if (type == REDIR_OUT)
		return (">");
	if (type == REDIR_APPEND)
		return (">>");
	if (type == HEREDOC)
		return ("<<");
	return ("?");
}

void	logger_tokens(t_list *tokens, const char *tag)
{
	t_list	*node;
	t_token	*token;

	if (LOG != 1)
		return ;
	printf("[TOKENS] %s\n", tag ? tag : "(no tag)");
	node = tokens;
	while (node)
	{
		token = (t_token *)node->content;
		if (token)
		{
			printf("  id=%d type=%s value=\"%s\" syntax=%d\n",
				token->id, token_type_to_str(token->type),
				token->value ? token->value : "(null)",
				token->syntax_error);
		}
		node = node->next;
	}
}

void	logger_commands(t_list *commands, const char *tag)
{
	t_list		*node;
	t_command	*cmd;
	t_list		*sub;
	t_prefix	*prefix;
	t_suffix	*suffix;
	int			index;

	if (LOG != 1)
		return ;
	printf("[COMMANDS] %s\n", tag ? tag : "(no tag)");
	node = commands;
	index = 0;
	while (node)
	{
		cmd = (t_command *)node->content;
		printf("  cmd[%d] word=\"%s\"\n", index,
			cmd && cmd->cmd_word ? cmd->cmd_word : "(null)");
		if (cmd)
		{
			sub = cmd->cmd_prefix;
			while (sub)
			{
				prefix = (t_prefix *)sub->content;
				if (prefix && prefix->assignment_word)
					printf("    prefix assign=\"%s\"\n",
						prefix->assignment_word);
				if (prefix && prefix->io_redirect)
				{
					if (prefix->io_redirect->io_file)
						printf("    prefix redir %s target=\"%s\"\n",
							redir_type_to_str(prefix->io_redirect->io_file->type),
							prefix->io_redirect->io_file->filename);
					if (prefix->io_redirect->io_here)
						printf("    prefix redir << target=\"%s\"\n",
							prefix->io_redirect->io_here->here_end);
				}
				sub = sub->next;
			}
			sub = cmd->cmd_suffix;
			while (sub)
			{
				suffix = (t_suffix *)sub->content;
				if (suffix && suffix->word)
					printf("    suffix word=\"%s\"\n", suffix->word);
				if (suffix && suffix->io_redirect)
				{
					if (suffix->io_redirect->io_file)
						printf("    suffix redir %s target=\"%s\"\n",
							redir_type_to_str(suffix->io_redirect->io_file->type),
							suffix->io_redirect->io_file->filename);
					if (suffix->io_redirect->io_here)
						printf("    suffix redir << target=\"%s\"\n",
							suffix->io_redirect->io_here->here_end);
				}
				sub = sub->next;
			}
		}
		node = node->next;
		index++;
	}
}

void	logger_open_fds(const char *starttag, const char *endtag)
{
	DIR			*dir;
	struct dirent	*entry;
	char			path[256];
	char			link_target[256];
	ssize_t		len;

	if (LOG != 1)
		return ;
	printf("[OPEN_FDS] %s\n", starttag ? starttag : "(start)");
	dir = opendir("/proc/self/fd");
	if (!dir)
	{
		printf("  (failed to open /proc/self/fd)\n");
		printf("[OPEN_FDS] %s\n", endtag ? endtag : "(end)");
		return ;
	}
	entry = readdir(dir);
	while (entry)
	{
		if (entry->d_name[0] != '.')
		{
			snprintf(path, sizeof(path), "/proc/self/fd/%s", entry->d_name);
			len = readlink(path, link_target, sizeof(link_target) - 1);
			if (len >= 0)
			{
				link_target[len] = '\0';
				printf("  fd %s -> %s\n", entry->d_name, link_target);
			}
			else
				printf("  fd %s\n", entry->d_name);
		}
		entry = readdir(dir);
	}
	closedir(dir);
	printf("[OPEN_FDS] %s\n", endtag ? endtag : "(end)");
}

void	logger_argv(char **argv, const char *tag)
{
	int	idx;

	if (LOG != 1)
		return ;
	printf("[ARGV] %s\n", tag ? tag : "(no tag)");
	if (!argv)
	{
		printf("  (null)\n");
		return ;
	}
	idx = 0;
	while (argv[idx])
	{
		printf("  argv[%d]=\"%s\"\n", idx, argv[idx]);
		idx++;
	}
}

static const char	*redir_exec_type_to_str(t_redir_type type)
{
	if (type == R_IN)
		return ("<");
	if (type == R_OUT_TRUNC)
		return (">");
	if (type == R_OUT_APPEND)
		return (">>");
	if (type == R_HEREDOC)
		return ("<<");
	return ("?");
}

void	logger_exec_cmds(t_list *exec_cmds, const char *tag)
{
	t_list	*node;
	t_cmd	*cmd;
	t_list	*redir_node;
	t_redir	*redir;
	int		idx;
	int		aidx;

	if (LOG != 1)
		return ;
	printf("[EXEC] %s\n", tag ? tag : "(no tag)");
	node = exec_cmds;
	idx = 0;
	while (node)
	{
		cmd = (t_cmd *)node->content;
		printf("  cmd[%d]\n", idx);
		if (!cmd)
		{
			printf("    (null)\n");
			node = node->next;
			idx++;
			continue ;
		}
		if (!cmd->argv)
			printf("    argv=(null)\n");
		else
		{
			aidx = 0;
			while (cmd->argv[aidx])
			{
				printf("    argv[%d]=\"%s\"\n", aidx, cmd->argv[aidx]);
				aidx++;
			}
		}
		redir_node = cmd->redirs;
		if (!redir_node)
			printf("    redirs=(null)\n");
		while (redir_node)
		{
			redir = (t_redir *)redir_node->content;
			if (redir)
			{
				printf("    redir type=%s fd=%d target=\"%s\" quoted=%d\n",
					redir_exec_type_to_str(redir->type), redir->fd,
					redir->target ? redir->target : "(null)",
					redir->quoted);
			}
			redir_node = redir_node->next;
		}
		node = node->next;
		idx++;
	}
}

// Source - https://stackoverflow.com/a
// Posted by Edwin Buck, modified by community.
// See post 'Timeline' for change history
// Retrieved 2026-01-13, License - CC BY-SA 3.0