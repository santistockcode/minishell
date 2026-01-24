#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "../../../include/exec.h"
#include "../../../include/minishell.h"

char **dup_argv(const char *const *argv_in, size_t argc);
t_cmd  *new_cmd_from_args(const char *const *argv_in, size_t argc);
t_redir *make_redir(t_redir_type type, const char *target, int quoted, int fd);

t_list *deep_copy_env(const char **envp);
void    free_env_struct(void *env);
void    free_shell(t_shell *sh);
t_shell *create_test_shell(const char **test_env, int last_status);


int		msh_apply_redirs(int *save_in, int *save_out, int *save_err);

void			msh_restore_stdio(int save_in, int save_out, int save_err);

#endif