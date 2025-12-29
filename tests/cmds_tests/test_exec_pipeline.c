#include <criterion/criterion.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/exec.h"
#include "../../include/minishell.h"
#include "../../Libft/include/libft.h"

// static char **dup_argv(const char *const *argv_in, size_t argc)
// {
//     char **argv = (char**)malloc(sizeof(char*) * (argc + 1));
//     if (!argv) return NULL;
//     for (size_t i = 0; i < argc; ++i) {
//         size_t len = strlen(argv_in[i]);
//         argv[i] = (char*)malloc(len + 1);
//         if (!argv[i]) {
//             for (size_t j = 0; j < i; ++j) free(argv[j]);
//             free(argv);
//             return NULL;
//         }
//         memcpy(argv[i], argv_in[i], len + 1);
//     }
//     argv[argc] = NULL;
//     return argv;
// }

// static t_cmd *make_cmd(const char *const *argv_in, size_t argc)
// {
//     t_cmd *cmd = (t_cmd*)malloc(sizeof(t_cmd));
//     if (!cmd) return NULL;
//     cmd->argv = dup_argv(argv_in, argc);
//     if (!cmd->argv) { free(cmd); return NULL; }
//     cmd->redirs = NULL;
//     return cmd;
// }

// static void add_redir(t_cmd *cmd, t_redir_type type, int fd, const char *target, int quoted)
// {
//     if (!cmd) return;
//     t_redir *r = (t_redir*)malloc(sizeof(t_redir));
//     if (!r) return;
//     r->type = type;
//     r->fd = fd;
//     r->quoted = quoted;
//     if (target) {
//         size_t len = strlen(target);
//         r->target = (char*)malloc(len + 1);
//         if (r->target) memcpy(r->target, target, len + 1);
//     } else {
//         r->target = NULL;
//     }
//     t_list *node = ft_lstnew(r);
//     if (!node) { free(r->target); free(r); return; }
//     ft_lstadd_back(&cmd->redirs, node);
// }

// static t_list *pipeline_append(t_list *head, t_cmd *cmd)
// {
//     t_list *node = ft_lstnew(cmd);
//     if (!node) return head;
//     ft_lstadd_back(&head, node);
//     return head;
// }

TestSuite(exec_cmds);

// Test(exec_cmds, redirs_in_and_out_append_returns_zero)
// {
//     t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
//     cr_assert_neq(sh, NULL, "malloc shell failed");
//     sh->i = 0;
//     sh->last_status = 0;
//     sh->should_exit = 0;

//     /* Build: wc < infile | wc >> outfile */
//     const char *argv1[] = {"wc"};
//     const char *argv2[] = {"wc"};

//     t_cmd *c1 = make_cmd(argv1, 1);
//     t_cmd *c2 = make_cmd(argv2, 1);
//     cr_assert_neq(c1, NULL, "cmd1 alloc failed");
//     cr_assert_neq(c2, NULL, "cmd2 alloc failed");

//     add_redir(c1, R_IN, 0, "infile", 0);
//     add_redir(c2, R_OUT_APPEND, 1, "outfile", 0);

//     t_list *pipe_head = NULL;
//     pipe_head = pipeline_append(pipe_head, c1);
//     pipe_head = pipeline_append(pipe_head, c2);

//     int rc = exec_cmds(sh, pipe_head);
//     cr_assert_eq(rc, 0, "exec_cmds should return 0 for valid pipeline");

//     free_pipeline(pipe_head);
//     free(sh);
// }

Test(exec_cmds, empty_pipeline_returns_zero)
{
    t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
    cr_assert_neq(sh, NULL, "malloc shell failed");
    sh->i = 0;
    sh->last_status = 0;
    sh->should_exit = 0;

    t_list *pipe_head = NULL;
    int rc = exec_cmds(sh, pipe_head);
    cr_assert_eq(rc, 0, "exec_cmds should return 0 for empty pipeline");
    free(sh);
}
