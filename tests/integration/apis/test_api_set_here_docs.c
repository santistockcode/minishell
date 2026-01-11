/* ************************************************************************** */
/*                                                                            */
/*   test_api_heredoc.c                                                       */
/*                                                                            */
/*   Build a real t_shell + (t_list of t_cmd) from simple specs.              */
/*   Then call set_here_docs and expose getters for Python.                   */
/*                                                                            */
/* ************************************************************************** */

#include "test_api_set_here_docs.h"
#include "../../include/minishell.h"
#include "../../include/exec.h"
#include <stdlib.h>
#include <string.h>

/* You told me you already have this: */
void	free_cmds(t_list *cmd_first);

/* ----------------------------- small helpers ----------------------------- */

static size_t	msh_strlen(const char *s)
{
	size_t i = 0;
	if (!s) return 0;
	while (s[i]) i++;
	return i;
}

static char	*msh_strdup(const char *s)
{
	size_t	len;
	char	*dup;
	size_t	i;

	if (!s)
		return NULL;
	len = msh_strlen(s);
	dup = (char *)malloc(len + 1);
	if (!dup)
		return NULL;
	i = 0;
	while (i < len)
	{
		dup[i] = s[i];
		i++;
	}
	dup[i] = '\0';
	return dup;
}

/* We implement our own list helpers to avoid symbol clashes with libft */
static t_list	*msh_lstnew(void *content)
{
	t_list	*n;

	n = (t_list *)malloc(sizeof(t_list));
	if (!n)
		return NULL;
	n->content = content;
	n->next = NULL;
	return n;
}

static void	msh_lstadd_back(t_list **lst, t_list *new_node)
{
	t_list	*it;

	if (!lst || !new_node)
		return;
	if (!*lst)
	{
		*lst = new_node;
		return;
	}
	it = *lst;
	while (it->next)
		it = it->next;
	it->next = new_node;
}

static int	msh_count_list(t_list *lst)
{
	int n = 0;
	while (lst)
	{
		n++;
		lst = lst->next;
	}
	return n;
}

/* ----------------------------- env handling ------------------------------ */

static t_env	*env_kv_from_envp_entry(const char *kv)
{
	const char	*eq;
	t_env		*e;
	size_t		klen;
	size_t		vlen;
	char		*k;
	char		*v;

	if (!kv)
		return NULL;
	eq = strchr(kv, '=');
	if (!eq)
		return NULL;

	klen = (size_t)(eq - kv);
	vlen = msh_strlen(eq + 1);

	k = (char *)malloc(klen + 1);
	v = (char *)malloc(vlen + 1);
	e = (t_env *)malloc(sizeof(t_env));
	if (!k || !v || !e)
	{
		free(k); free(v); free(e);
		return NULL;
	}
	memcpy(k, kv, klen);
	k[klen] = '\0';
	memcpy(v, eq + 1, vlen);
	v[vlen] = '\0';

	e->key = k;
	e->value = v;
	return e;
}

static void	free_env_list(t_list *env)
{
	t_list	*next;
	t_env	*e;

	while (env)
	{
		next = env->next;
		e = (t_env *)env->content;
		if (e)
		{
			free(e->key);
			free(e->value);
			free(e);
		}
		free(env);
		env = next;
	}
}

static t_list	*env_list_from_envp(char **envp)
{
	t_list	*env = NULL;
	int		i;

	if (!envp)
		return NULL;
	i = 0;
	while (envp[i])
	{
		t_env	*e = env_kv_from_envp_entry(envp[i]);
		t_list	*n;

		if (!e)
		{
			free_env_list(env);
			return NULL;
		}
		n = msh_lstnew(e);
		if (!n)
		{
			free(e->key); free(e->value); free(e);
			free_env_list(env);
			return NULL;
		}
		msh_lstadd_back(&env, n);
		i++;
	}
	return env;
}

/* ----------------------------- cmd building ------------------------------ */

static t_redir	*redir_from_spec(const t_msh_test_redir_spec *s)
{
	t_redir	*r;

	r = (t_redir *)malloc(sizeof(t_redir));
	if (!r)
		return NULL;
	r->type = (t_redir_type)s->type;
	r->fd = s->fd;
	r->target = msh_strdup(s->target);
	r->quoted = s->quoted;
	if (s->target && !r->target)
	{
		free(r);
		return NULL;
	}
	return r;
}

static t_list	*redir_list_from_specs(const t_msh_test_redir_spec *specs)
{
	t_list	*lst = NULL;
	int		i;

	if (!specs)
		return NULL;
	i = 0;
	while (specs[i].type != -1)
	{
		t_redir	*r = redir_from_spec(&specs[i]);
		t_list	*n;

		if (!r)
		{
			/* best-effort cleanup using your project’s free routines:
			   easiest is to build a dummy cmd and reuse free_cmds, but
			   we keep it simple here: just leak-minimal by freeing list manually
			   is harder because your redirs are t_redir*; we’ll do manual free. */
			/* manual cleanup */
			while (lst)
			{
				t_list *nx = lst->next;
				t_redir *rr = (t_redir *)lst->content;
				if (rr) { free(rr->target); free(rr); }
				free(lst);
				lst = nx;
			}
			return NULL;
		}
		n = msh_lstnew(r);
		if (!n)
		{
			free(r->target);
			free(r);
			while (lst)
			{
				t_list *nx = lst->next;
				t_redir *rr = (t_redir *)lst->content;
				if (rr) { free(rr->target); free(rr); }
				free(lst);
				lst = nx;
			}
			return NULL;
		}
		msh_lstadd_back(&lst, n);
		i++;
	}
	return lst;
}

static char	**argv_dup(const char **argv)
{
	int		count;
	char	**out;
	int		i;

	if (!argv)
		return NULL;
	count = 0;
	while (argv[count])
		count++;

	out = (char **)calloc((size_t)count + 1, sizeof(char *));
	if (!out)
		return NULL;

	i = 0;
	while (i < count)
	{
		out[i] = msh_strdup(argv[i]);
		if (!out[i])
		{
			/* cleanup */
			while (i > 0)
			{
				i--;
				free(out[i]);
			}
			free(out);
			return NULL;
		}
		i++;
	}
	out[count] = NULL;
	return out;
}

static t_cmd	*cmd_from_spec(const t_msh_test_cmd_spec *s)
{
	t_cmd	*c;

	c = (t_cmd *)malloc(sizeof(t_cmd));
	if (!c)
		return NULL;
	c->argv = argv_dup(s->argv);
	c->redirs = redir_list_from_specs(s->redirs);
	/* If argv_dup fails but redirs alloc succeeded, free_cmds will clean it,
	   but here we’re still building: handle both cases. */
	if (s->argv && !c->argv)
	{
		/* free redirs manually */
		t_list *r = c->redirs;
		while (r)
		{
			t_list *nx = r->next;
			t_redir *rr = (t_redir *)r->content;
			if (rr) { free(rr->target); free(rr); }
			free(r);
			r = nx;
		}
		free(c);
		return NULL;
	}
	return c;
}

static t_list	*cmds_list_from_specs(const t_msh_test_cmd_spec *cmds, int cmd_count)
{
	t_list	*lst = NULL;
	int		i;

	if (!cmds || cmd_count <= 0)
		return NULL;
	i = 0;
	while (i < cmd_count)
	{
		t_cmd	*c = cmd_from_spec(&cmds[i]);
		t_list	*n;

		if (!c)
		{
			free_cmds(lst);
			return NULL;
		}
		n = msh_lstnew(c);
		if (!n)
		{
			/* let your existing cleanup do the heavy lifting */
			/* We need to wrap c into a list to use free_cmds consistently */
			t_list *tmp = msh_lstnew(c);
			if (tmp)
				free_cmds(tmp);
			else
			{
				/* fallback */
				free(c);
			}
			free_cmds(lst);
			return NULL;
		}
		msh_lstadd_back(&lst, n);
		i++;
	}
	return lst;
}

/* --------------------------- exported test ctx --------------------------- */

typedef struct s_msh_test_ctx
{
	t_shell	sh;
	t_list	*cmd_first;
	int		cmd_count;
}	t_msh_test_ctx;

void	*msh_test_ctx_create(const t_msh_test_cmd_spec *cmds,
			int cmd_count, char **envp, int last_status)
{
	t_msh_test_ctx	*ctx;

	ctx = (t_msh_test_ctx *)calloc(1, sizeof(t_msh_test_ctx));
	if (!ctx)
		return NULL;

	ctx->sh.env = env_list_from_envp(envp);
	ctx->sh.last_status = last_status;
	ctx->sh.should_exit = 0;
	ctx->sh.last_err_op = NULL;

	ctx->cmd_first = cmds_list_from_specs(cmds, cmd_count);
	ctx->cmd_count = msh_count_list(ctx->cmd_first);

	if (cmd_count > 0 && !ctx->cmd_first)
	{
		free_env_list(ctx->sh.env);
		free(ctx);
		return NULL;
	}
	return (void *)ctx;
}

int	msh_test_set_here_docs(void *ctx_void)
{
	t_msh_test_ctx	*ctx;

	ctx = (t_msh_test_ctx *)ctx_void;
	if (!ctx)
		return -1;
	return set_here_docs(&ctx->sh, ctx->cmd_first);
}

const char	*msh_test_get_last_err_op(void *ctx_void)
{
	t_msh_test_ctx	*ctx = (t_msh_test_ctx *)ctx_void;
	if (!ctx)
		return NULL;
	return ctx->sh.last_err_op;
}

int	msh_test_get_last_status(void *ctx_void)
{
	t_msh_test_ctx	*ctx = (t_msh_test_ctx *)ctx_void;
	if (!ctx)
		return 0;
	return ctx->sh.last_status;
}

int	msh_test_get_cmd_count(void *ctx_void)
{
	t_msh_test_ctx	*ctx = (t_msh_test_ctx *)ctx_void;
	if (!ctx)
		return 0;
	return ctx->cmd_count;
}

static t_cmd	*get_cmd_at(t_list *cmd_first, int cmd_index)
{
	int i = 0;
	while (cmd_first)
	{
		if (i == cmd_index)
			return (t_cmd *)cmd_first->content;
		i++;
		cmd_first = cmd_first->next;
	}
	return NULL;
}

static t_redir	*get_redir_at(t_list *redirs, int redir_index)
{
	int i = 0;
	while (redirs)
	{
		if (i == redir_index)
			return (t_redir *)redirs->content;
		i++;
		redirs = redirs->next;
	}
	return NULL;
}

const char	*msh_test_get_redir_target(void *ctx_void, int cmd_index, int redir_index)
{
	t_msh_test_ctx	*ctx = (t_msh_test_ctx *)ctx_void;
	t_cmd			*cmd;
	t_redir			*r;

	if (!ctx)
		return NULL;
	cmd = get_cmd_at(ctx->cmd_first, cmd_index);
	if (!cmd)
		return NULL;
	r = get_redir_at(cmd->redirs, redir_index);
	if (!r)
		return NULL;
	return r->target;
}

int	msh_test_get_redir_type(void *ctx_void, int cmd_index, int redir_index)
{
	t_msh_test_ctx	*ctx = (t_msh_test_ctx *)ctx_void;
	t_cmd			*cmd;
	t_redir			*r;

	if (!ctx)
		return -1;
	cmd = get_cmd_at(ctx->cmd_first, cmd_index);
	if (!cmd)
		return -1;
	r = get_redir_at(cmd->redirs, redir_index);
	if (!r)
		return -1;
	return (int)r->type;
}

int	msh_test_get_redir_fd(void *ctx_void, int cmd_index, int redir_index)
{
	t_msh_test_ctx	*ctx = (t_msh_test_ctx *)ctx_void;
	t_cmd			*cmd;
	t_redir			*r;

	if (!ctx)
		return -1;
	cmd = get_cmd_at(ctx->cmd_first, cmd_index);
	if (!cmd)
		return -1;
	r = get_redir_at(cmd->redirs, redir_index);
	if (!r)
		return -1;
	return r->fd;
}

int	msh_test_get_redir_quoted(void *ctx_void, int cmd_index, int redir_index)
{
	t_msh_test_ctx	*ctx = (t_msh_test_ctx *)ctx_void;
	t_cmd			*cmd;
	t_redir			*r;

	if (!ctx)
		return -1;
	cmd = get_cmd_at(ctx->cmd_first, cmd_index);
	if (!cmd)
		return -1;
	r = get_redir_at(cmd->redirs, redir_index);
	if (!r)
		return -1;
	return r->quoted;
}

void	msh_test_ctx_destroy(void *ctx_void)
{
	t_msh_test_ctx	*ctx;

	ctx = (t_msh_test_ctx *)ctx_void;
	if (!ctx)
		return;

	/* cmd list + its content */
	free_cmds(ctx->cmd_first);

	/* env list + its content */
	free_env_list(ctx->sh.env);

	/* last_err_op ownership depends on your design:
	   if you strdup into it, free it here. If it's a static pointer, don’t.
	   Most minishells strdup error op strings, so we free as safest default. */
	free(ctx->sh.last_err_op);

	free(ctx);
}
