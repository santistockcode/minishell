/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_api_exec.h.                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by s list in C       #+#    #+#             */
/*   Updated: 2026/01/09 16:12:56 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEST_API_EXEC_H
# define TEST_API_EXEC_H

# include <stddef.h>

/*
** Keep these “spec” types FFI-friendly:
** - ints + const char* + pointers
** - arrays terminated with a sentinel (type == -1 for redirs)
*/
typedef struct s_msh_test_redir_spec
{
	int         type;    /* matches your t_redir_type values */
	int         fd;
	const char  *target; /* delimiter for heredoc, or filename for others */
	int         quoted;
}	t_msh_test_redir_spec;

typedef struct s_msh_test_cmd_spec
{
	const char                **argv;   /* NULL-terminated */
	const t_msh_test_redir_spec *redirs;/* array terminated by { .type = -1 } */
}	t_msh_test_cmd_spec;

/* Opaque context handle for Python */
void	*msh_test_ctx_create(const t_msh_test_cmd_spec *cmds,
			int cmd_count, char **envp, int last_status);

/* Calls your real function: set_here_docs(&ctx->sh, ctx->cmd_first) */
int		msh_test_set_here_docs(void *ctx);

/* Accessors to inspect results after set_here_docs (e.g., heredoc paths) */
const char	*msh_test_get_last_err_op(void *ctx);
int			msh_test_get_last_status(void *ctx);

int			msh_test_get_cmd_count(void *ctx);

/*
** Returns NULL if out of range.
** Useful if your heredoc logic overwrites redir->target with generated path.
*/
const char	*msh_test_get_redir_target(void *ctx, int cmd_index, int redir_index);
int			msh_test_get_redir_type(void *ctx, int cmd_index, int redir_index);
int			msh_test_get_redir_fd(void *ctx, int cmd_index, int redir_index);
int			msh_test_get_redir_quoted(void *ctx, int cmd_index, int redir_index);

/* Cleanup (calls your free_cmds + frees env + ctx itself) */
void		msh_test_ctx_destroy(void *ctx);

#endif
