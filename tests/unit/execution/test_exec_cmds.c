#include <stdlib.h>
#include <string.h>

#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

#include "../../../include/exec.h"
#include "../../../include/minishell.h"
#include "../../../include/syswrap.h"
#include "../../../Libft/include/libft.h"

volatile sig_atomic_t g_exit_status = 0;

/*
Actions of exec cmds: 
    - evolve cmds list to manage here_docs (already tested)
    - execute commands (pending)
*/

static int test_empty_pipeline_returns_zero(void)
{
    printf("Test: test_empty_pipeline_returns_zero\n");
    t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
    mu_assert("malloc shell failed", sh != NULL);
    sh->last_status = 0;
    sh->should_exit = 0;

    t_list *pipe_head = NULL;
    int rc = exec_cmds(sh, pipe_head);
    mu_assert("exec_cmds should return 0 for empty pipeline", rc == 0);
    free(sh);
    return 0;
}

static int test_on_signal_c_last_status_set_to_130_when_set_here_docs(void)
{
    printf("Test: test_on_signal_c_last_status_set_to_130\n");
    t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
    mu_assert("malloc shell failed", sh != NULL);
    sh->last_status = 0;
    sh->last_err_op = NULL;
    sh->should_exit = 0;

    const char *argv1[] = {"wc"};
    const char *argv2[] = {"ls"};
    
    // cmd1: wc -l << EOF
    t_redir *redir1 = make_redir(R_HEREDOC, (char*)"EOF", 0, 0);
    t_cmd *cmd1 = new_cmd_from_args(argv1, 1);
    ft_lstadd_back(&cmd1->redirs, ft_lstnew(redir1));
    
    // cmd2: ls
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    
    t_list *pipe_head = NULL;
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));

    // Simulate Ctrl-C signal during here_doc setup
    g_exit_status = 130; // Simulate signal interrupt

    exec_cmds(sh, pipe_head);
    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    mu_assert("on signal C, last_status should be set to 130", sh->last_status == 130);

    free(sh);
    return 0;
}

int main(void)
{
    mu_run_test(test_empty_pipeline_returns_zero);
    mu_run_test(test_on_signal_c_last_status_set_to_130_when_set_here_docs);
    mu_summary();
}
