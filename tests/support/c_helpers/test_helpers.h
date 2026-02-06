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

t_list *create_middle_cmd_list_with_redir_in_on_second(void);
t_list *create_middle_cmd_list(void);
t_list *create_middle_cmd_list_with_redir_out_on_third(void);
void wait_for_all_children(int *statuses, int count);
int create_mock_pipe_with_data(const char *data);

#endif