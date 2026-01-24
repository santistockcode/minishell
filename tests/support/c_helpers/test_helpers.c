#include "test_helpers.h"
#include "../../Libft/include/libft.h"
#include "../../include/minishell.h"
#include <stdlib.h>
#include <string.h>

/* duplicate argv strings and allocate argv array on heap */
char **dup_argv(const char *const *argv_in, size_t argc)
{
    char **argv;
    size_t i;
    size_t j;
    
    i = 0;
    j = 0;
    argv = malloc(sizeof(char*) * (argc + 1));
    if (!argv) return NULL;
    while(i < argc)
    {
        argv[i] = ft_strdup(argv_in[i]);
        if (!argv[i]) {
            while(j < i)
                free(argv[j++]);
            return (free(argv), NULL);
        }
        i++;
    }
    argv[argc] = NULL;
    return argv;
}

/* create t_cmd owning a heap argv (use dup_argv) */
t_cmd *new_cmd_from_args(const char *const *argv_in, size_t argc)
{
    t_cmd *cmd = malloc(sizeof(*cmd));
    if (!cmd) return NULL;
    cmd->argv = (char **) dup_argv(argv_in, argc);
    cmd->redirs = NULL;
    cmd->stage_io = NULL;
    return cmd;
}

t_redir *make_redir(t_redir_type type, const char *target, int quoted, int fd)
{
    t_redir *redir = malloc(sizeof(*redir));
    if (!redir) return NULL;
    redir->type = type;
    redir->fd = fd;
    redir->target = target ? ft_strdup(target) : NULL;
    redir->quoted = quoted;
    return redir;
}

/* environment helpers */
t_list *deep_copy_env(const char **envp)
{
    t_list *env_list = NULL;
    for (int i = 0; envp && envp[i]; ++i) {
        char *eq = ft_strchr(envp[i], '=');
        if (!eq) continue;
        t_env *e = malloc(sizeof(*e));
        if (!e) continue;
        e->key = ft_substr(envp[i], 0, eq - envp[i]);
        e->value = ft_strdup(eq + 1);
        ft_lstadd_back(&env_list, ft_lstnew(e));
    }
    return env_list;
}

void free_env_struct(void *env)
{
    if (!env) return;
    free(((t_env*)env)->key);
    free(((t_env*)env)->value);
    free(env);
}

void free_shell(t_shell *sh)
{
    if (!sh) return;
    ft_lstclear(&sh->env, free_env_struct);
    if (sh->last_err_op)
        free(sh->last_err_op);
    free(sh);
}


t_shell *create_test_shell(const char **test_env, int last_status)
{
    t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
    if (!sh)
        return NULL;

    sh->last_status = last_status;
    sh->should_exit = 0;
    sh->env = deep_copy_env(test_env);
    sh->last_err_op = NULL;
    sh->save_in = -1;
    sh->save_out = -1;
    sh->save_err = -1;
    return sh;
}
