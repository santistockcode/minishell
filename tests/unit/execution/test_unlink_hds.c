#include <stdlib.h>
#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

#include "../../../include/exec.h"
#include "../../../include/minishell.h"
#include "../../../include/syswrap.h"
#include "../../../Libft/include/libft.h"

volatile sig_atomic_t exit_status = 0;


static int test_assert_here_docs_unlinked(void)
{
    printf("Test: test_assert_here_docs_unlinked\n");
    const char *argv1[] = {"echo", "hello"};
    const char *argv2[] = {"ls"};

    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    mu_assert("cmd1 alloc failed", cmd1 != NULL);
    t_redir *redir1 = make_redir(R_HEREDOC, ".here_doc_0", 1, 3);
    ft_lstadd_back(&cmd1->redirs, ft_lstnew(redir1));

    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    t_redir *redir2 = make_redir(R_HEREDOC, ".here_doc_1", 0, 4);
    ft_lstadd_back(&cmd2->redirs, ft_lstnew(redir2));
    mu_assert("cmd2 alloc failed", cmd2 != NULL);

    t_list *pipe_head = NULL;
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));
    //p ((t_cmd *)(*(t_list *)pipe_head).content)->argv[0]

	int fd1 = open(".here_doc_0", O_CREAT | O_WRONLY | O_TRUNC, 0644);
	int fd2 = open(".here_doc_1", O_CREAT | O_WRONLY | O_TRUNC, 0644);

    // unlink heredocs
    unlink_hds(pipe_head);

    close(fd1);
    close(fd2);

    // assert here_docs are unlinked
    mu_assert("here_doc_0 not unlinked", access(".here_doc_0", F_OK) == -1);
    mu_assert("here_doc_1 not unlinked", access(".here_doc_1", F_OK) == -1);

    /* free the pipeline (should free cmds, argv and nodes) */
    free_cmds(pipe_head);

    /* if we reached this point, free_cmds didn't crash — test passes */
    mu_assert("free_cmds completed", 1 == 1);
    return 0;
}

int main(void)
{
    mu_run_test(test_assert_here_docs_unlinked);
    mu_summary();
    return 0;
}