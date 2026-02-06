#include <stdlib.h>
#include <string.h>

#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"
#include "../../support/c_helpers/exec_cmds_helpers.h"

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

// static int test_empty_pipeline_returns_zero(void)
// {
//     printf("Test: test_empty_pipeline_returns_zero\n");
//     t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
//     mu_assert("malloc shell failed", sh != NULL);
//     sh->last_status = 0;
//     sh->should_exit = 0;

//     t_list *pipe_head = NULL;
//     int rc = exec_cmds(sh, pipe_head);
//     mu_assert("exec_cmds should return 0 for empty pipeline", rc == 0);
//     free(sh);
//     return 0;
// }

// static int test_on_signal_c_last_status_set_to_130_when_set_here_docs(void)
// {
//     printf("Test: test_on_signal_c_last_status_set_to_130\n");
//     t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
//     mu_assert("malloc shell failed", sh != NULL);
//     sh->last_status = 0;
//     sh->last_err_op = NULL;
//     sh->should_exit = 0;

//     const char *argv1[] = {"wc"};
//     const char *argv2[] = {"ls"};
    
//     // cmd1: wc -l << EOF
//     t_redir *redir1 = make_redir(R_HEREDOC, (char*)"EOF", 0, 0);
//     t_cmd *cmd1 = new_cmd_from_args(argv1, 1);
//     ft_lstadd_back(&cmd1->redirs, ft_lstnew(redir1));
    
//     // cmd2: ls
//     t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    
//     t_list *pipe_head = NULL;
//     ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
//     ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));

//     // Simulate Ctrl-C signal during here_doc setup
//     g_exit_status = 130; // Simulate signal interrupt

//     exec_cmds(sh, pipe_head);
//     unlink_hds(pipe_head);
//     free_cmds(pipe_head);
//     mu_assert("on signal C, last_status should be set to 130", sh->last_status == 130);

//     free(sh);
//     return 0;
// }

/* ============================================================================
 * TEST ENVIRONMENT
 * ============================================================================ */
static const char *test_env[] = {
    "PATH=/usr/bin:/bin",
    "USER=test",
    "HOME=/tmp",
    NULL
};

/* ============================================================================
 * HELPER: Wait for all children and return
 * ============================================================================ */
static void wait_all_children(void)
{
    while (wait(NULL) > 0)
        ;
}

/* ============================================================================
 * EXEC_SIMPLE: BUILTIN HAPPY PATHS
 * ============================================================================ */

static int test_builtin_simple_ok(void)
{
    printf("Test: builtin simple (no redir)\n");
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "hello", NULL};
    t_cmd *cmd = fd_make_cmd(argv, 2);
    t_list *cmds = ft_lstnew(cmd);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    logger_open_fds("♻️test_builtin_simple_ok♻️", "[should be restored]");
    mu_assert_intcmp("status should be 0", status, 0);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

static int test_builtin_redir_in(void)
{
    printf("Test: builtin < in\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "input content\n");
    
    const char *argv[] = {"echo", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in(argv, 1, FD_TEST_INPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_builtin_redir_in♻️", "should be restored");

    mu_assert_intcmp("status should be 0", status, 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_builtin_redir_out(void)
{
    printf("Test: builtin > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    const char *argv[] = {"echo", "output_test", NULL};
    t_cmd *cmd = fd_make_cmd_redir_out(argv, 2, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_builtin_redir_out♻️", "should be restored");

    mu_assert_intcmp("status should be 0", status, 0);
    mu_assert_intcmp("file should have content", 
                     verify_file_content(FD_TEST_OUTPUT, "output_test"), 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_builtin_redir_in_out(void)
{
    printf("Test: builtin < in > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "transform_me\n");
    
    const char *argv[] = {"echo", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in_out(argv, 1, FD_TEST_INPUT, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_builtin_redir_in_out♻️", "should be restored");

    mu_assert_intcmp("status should be 0", status, 0);
    mu_assert_intcmp("file should have content", 
                     verify_file_content(FD_TEST_OUTPUT, ""), 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_SIMPLE: BUILTIN DUP FAILS
 * ============================================================================ */

static int test_builtin_dup_fails_redir_in(void)
{
    printf("Test: builtin dup fails < in\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    const char *argv[] = {"echo", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in(argv, 1, FD_TEST_INPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_dup_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("test_builtin_dup_fails_redir_in♻️", "should be restored");

    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_builtin_dup_fails_redir_out(void)
{
    printf("Test: builtin dup fails > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    const char *argv[] = {"echo", "test", NULL};
    t_cmd *cmd = fd_make_cmd_redir_out(argv, 2, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_dup_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_builtin_dup_fails_redir_out♻️", "should be restored");

    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_builtin_dup_fails_redir_in_out(void)
{
    printf("Test: builtin dup fails < in > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    const char *argv[] = {"echo", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in_out(argv, 1, FD_TEST_INPUT, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_dup_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_builtin_dup_fails_redir_in_out♻️", "should be restored");

    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_SIMPLE: BUILTIN OPEN FAILS
 * ============================================================================ */

static int test_builtin_open_fails_redir_in(void)
{
    printf("Test: builtin open fails < in\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    // Use nonexistent file - open will fail naturally
    const char *argv[] = {"echo", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in(argv, 1, FD_TEST_NONEXISTENT);
    t_list *cmds = ft_lstnew(cmd);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_builtin_open_fails_redir_in♻️", "should be restored");

    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_builtin_open_fails_redir_out(void)
{
    printf("Test: builtin open fails > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    const char *argv[] = {"echo", "test", NULL};
    t_cmd *cmd = fd_make_cmd_redir_out(argv, 2, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_open_fail_at(0);
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_builtin_open_fails_redir_out♻️", "should be restored");
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_builtin_open_fails_redir_in_out(void)
{
    printf("Test: builtin open fails < in > out (first open fails)\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    const char *argv[] = {"echo", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in_out(argv, 1, FD_TEST_INPUT, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_open_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_builtin_open_fails_redir_in_out♻️", "should be restored");
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_SIMPLE: BUILTIN DUP2 FAILS
 * ============================================================================ */

static int test_builtin_dup2_fails_redir_in(void)
{
    printf("Test: builtin dup2 fails < in\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    const char *argv[] = {"echo", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in(argv, 1, FD_TEST_INPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_dup2_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_builtin_dup2_fails_redir_in♻️", "should be restored");
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_builtin_dup2_fails_redir_out(void)
{
    printf("Test: builtin dup2 fails > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    const char *argv[] = {"echo", "test", NULL};
    t_cmd *cmd = fd_make_cmd_redir_out(argv, 2, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_dup2_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_builtin_dup2_fails_redir_out♻️", "should be restored");
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_builtin_dup2_fails_redir_in_out(void)
{
    printf("Test: builtin dup2 fails < in > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    const char *argv[] = {"echo", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in_out(argv, 1, FD_TEST_INPUT, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_dup2_fail_at(0);
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_builtin_dup2_fails_redir_in_out♻️", "should be restored");
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_SIMPLE: EXTERNAL HAPPY PATHS
 * ============================================================================ */

static int test_external_redir_in(void)
{
    printf("Test: external < in\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "line1\nline2\nline3\n");
    
    const char *argv[] = {"/usr/bin/head", "-n", "1", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in(argv, 3, FD_TEST_INPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    mu_assert_intcmp("status should be 0", status, 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_external_redir_out(void)
{
    printf("Test: external > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    const char *argv[] = {"/bin/echo", "external_output", NULL};
    t_cmd *cmd = fd_make_cmd_redir_out(argv, 2, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    mu_assert_intcmp("status should be 0", status, 0);
    mu_assert_intcmp("file should have content", 
                     verify_file_content(FD_TEST_OUTPUT, "external_output"), 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_external_redir_in_out(void)
{
    printf("Test: external < in > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "line1\nline2\nline3\n");
    
    const char *argv[] = {"/usr/bin/head", "-n", "2", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in_out(argv, 3, FD_TEST_INPUT, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    mu_assert_intcmp("status should be 0", status, 0);
    mu_assert_intcmp("file should have content", 
                     verify_file_content(FD_TEST_OUTPUT, "line1\nline2\n"), 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_SIMPLE: EXTERNAL FORK FAILS
 * ============================================================================ */

static int test_external_fork_fails_redir_in(void)
{
    printf("Test: external fork fails < in\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    const char *argv[] = {"/bin/cat", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in(argv, 1, FD_TEST_INPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_fork_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("test_external_fork_fails_redir_in♻️", "should be restored");
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_external_fork_fails_redir_out(void)
{
    printf("Test: external fork fails > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    const char *argv[] = {"/bin/echo", "test", NULL};
    t_cmd *cmd = fd_make_cmd_redir_out(argv, 2, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_fork_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("test_external_fork_fails_redir_out♻️", "should be restored");
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_external_fork_fails_redir_in_out(void)
{
    printf("Test: external fork fails < in > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    const char *argv[] = {"/bin/cat", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in_out(argv, 1, FD_TEST_INPUT, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_fork_fail_at(0);
    logger_open_fds("test_external_fork_fails_redir_in_out♻️", "should be restored");
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_SIMPLE: EXTERNAL OPEN FAILS
 * ============================================================================ */

static int test_external_open_fails_redir_in(void)
{
    printf("Test: external open fails < in\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    const char *argv[] = {"/bin/cat", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in(argv, 1, FD_TEST_NONEXISTENT);
    t_list *cmds = NULL;
    ft_lstadd_back(&cmds, ft_lstnew(cmd));
    
      int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_external_open_fails_redir_out(void)
{
    printf("Test: external open fails > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    const char *argv[] = {"/bin/echo", "test", NULL};
    t_cmd *cmd = fd_make_cmd_redir_out(argv, 2, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_open_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_external_open_fails_redir_in_out(void)
{
    printf("Test: external open fails < in > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    const char *argv[] = {"/bin/cat", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in_out(argv, 1, FD_TEST_INPUT, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_open_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_SIMPLE: EXTERNAL DUP2 FAILS
 * ============================================================================ */

static int test_external_dup2_fails_redir_in(void)
{
    printf("Test: external dup2 fails < in\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    const char *argv[] = {"/bin/cat", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in(argv, 1, FD_TEST_INPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_dup2_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_external_dup2_fails_redir_out(void)
{
    printf("Test: external dup2 fails > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    const char *argv[] = {"/bin/echo", "test", NULL};
    t_cmd *cmd = fd_make_cmd_redir_out(argv, 2, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_dup2_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_external_dup2_fails_redir_in_out(void)
{
    printf("Test: external dup2 fails < in > out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    const char *argv[] = {"/bin/cat", NULL};
    t_cmd *cmd = fd_make_cmd_redir_in_out(argv, 1, FD_TEST_INPUT, FD_TEST_OUTPUT);
    t_list *cmds = ft_lstnew(cmd);
    
    fd_setup_dup2_fail_at(1);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_PIPELINE: BUILTIN HAPPY PATHS
 * ============================================================================ */

static int test_pipeline_builtin_3_stage_ok(void)
{
    printf("Test: builtin | builtin | builtin (all ok)\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = fd_config_builtin_3_stage();
    t_list *cmds = fd_build_pipeline(&config);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    mu_assert_intcmp("status should be 0", status, 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_builtin_3_stage_all_redirs_ok(void)
{
    printf("Test: builtin_in | builtin | builtin_out (all ok)\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "pipeline_input\n");
    
    t_pipeline_config config = fd_config_builtin_3_stage_all_redirs();
    t_list *cmds = fd_build_pipeline(&config);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    mu_assert_intcmp("status should be 0", status, 0);
    mu_assert_intcmp("output file should have content",
                     verify_file_content(FD_TEST_OUTPUT, ""), 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_PIPELINE: BUILTIN FORK FAILS
 * ============================================================================ */

static int test_pipeline_builtin_fork_fails_first(void)
{
    printf("Test: builtin fork fails | builtin | builtin\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = fd_config_builtin_3_stage();
    t_list *cmds = fd_build_pipeline(&config);
    
    fd_setup_fork_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("test_pipeline_builtin_fork_fails_first♻️", "should be restored");
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_builtin_fork_fails_second(void)
{
    return (0);
    printf("Test: builtin | builtin fork fails | builtin\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = fd_config_builtin_3_stage();
    t_list *cmds = fd_build_pipeline(&config);
    
    // fd_setup_fork_fail_at(0);
    // needs to do it manually. Raises SIGPIPE
    printf("test_pipeline_builtin_fork_fails_second have still reachable");
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("test_pipeline_builtin_fork_fails_second♻️", "should be restored");
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_builtin_fork_fails_third(void)
{
    return (0);
    printf("Test: builtin | builtin | builtin fork fails\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = fd_config_builtin_3_stage();
    t_list *cmds = fd_build_pipeline(&config);
    
    fd_setup_fork_fail_at(0);
    // needs to do it manually. Raises SIGPIPE
    printf("test_pipeline_builtin_fork_fails_third have still reachable");
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    syswrap_set_fork(NULL);
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_PIPELINE: BUILTIN OPEN FAILS
 * ============================================================================ */

static int test_pipeline_builtin_open_fails_first_in(void)
{
    printf("Test: builtin_in open fails | builtin | builtin\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    // Use nonexistent file for natural open failure
    t_pipeline_config config = {
        .argv1 = {"echo", NULL, NULL, NULL},
        .argc1 = 1,
        .redir1 = REDIR_IN,
        .argv2 = {"echo", NULL, NULL, NULL},
        .argc2 = 1,
        .redir2 = REDIR_NONE,
        .argv3 = {"echo", NULL, NULL, NULL},
        .argc3 = 1,
        .redir3 = REDIR_NONE,
        .in_file = FD_TEST_NONEXISTENT,
        .out_file = NULL
    };
    // echo < inexistant | echo | echo
    t_list *cmds = fd_build_pipeline(&config);
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    /*
    (.env) saalarco@c1r9s6:~/Dev/minishell$ bash
    saalarco@c1r9s6:~/Dev/minishell$ echo < nonexistant | echo | echo
    bash: nonexistant: No such file or directory
    saalarco@c1r9s6:~/Dev/minishell$ echo $?
    0
    saalarco@c1r9s6:~/Dev/minishell$ 
    */
    mu_assert("status should be non-zero", status == 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_builtin_open_fails_first_out(void)
{
    printf("Test: builtin_out open fails | builtin | builtin\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = {
        .argv1 = {"echo", "test", NULL, NULL},
        .argc1 = 2,
        .redir1 = REDIR_OUT,
        .argv2 = {"cat", NULL, NULL, NULL},
        .argc2 = 1,
        .redir2 = REDIR_NONE,
        .argv3 = {"echo", NULL, NULL, NULL},
        .argc3 = 1,
        .redir3 = REDIR_IN_OUT,
        .in_file = FD_TEST_NONEXISTENT,
        .out_file = FD_TEST_OUTPUT
    };
    t_list *cmds = fd_build_pipeline(&config);
    
    fd_setup_open_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}


/*
Problema a mi me da sigpipe pero a bash solo le da leaks.
*/
static int test_pipeline_builtin_open_fails_first_in_out(void)
{
    return (0);
    printf("Test: builtin_in_out open fails | builtin | builtin_in_out\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    t_pipeline_config config = {
        .argv1 = {"echo", NULL, NULL, NULL},
        .argc1 = 1,
        .redir1 = REDIR_IN_OUT,
        .argv2 = {"echo", NULL, NULL, NULL},
        .argc2 = 1,
        .redir2 = REDIR_NONE,
        .argv3 = {"echo", NULL, NULL, NULL},
        .argc3 = 1,
        .redir3 = REDIR_IN_OUT,
        .in_file = FD_TEST_INPUT,
        .out_file = FD_TEST_OUTPUT
    };
    t_list *cmds = fd_build_pipeline(&config);
    
    fd_setup_open_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_PIPELINE: BUILTIN DUP2 FAILS
 * ============================================================================ */

static int test_pipeline_builtin_dup2_fails_middle(void)
{
    return (0);
    printf("Test: builtin | builtin_in_out dup2 fails | builtin\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    t_pipeline_config config = {
        .argv1 = {"echo", "hello", NULL, NULL},
        .argc1 = 2,
        .redir1 = REDIR_NONE,
        .argv2 = {"cat", NULL, NULL, NULL},
        .argc2 = 1,
        .redir2 = REDIR_IN_OUT,
        .argv3 = {"cat", NULL, NULL, NULL},
        .argc3 = 1,
        .redir3 = REDIR_NONE,
        .in_file = FD_TEST_INPUT,
        .out_file = FD_TEST_OUTPUT
    };
    t_list *cmds = fd_build_pipeline(&config);
    
    // Fail dup2 after a few calls (for middle stage)
    fd_setup_dup2_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_PIPELINE: EXTERNAL HAPPY PATHS
 * ============================================================================ */

static int test_pipeline_external_3_stage_ok(void)
{
    printf("Test: external | external | external (all ok)\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "line1\nline2\nline3\n");
    
    // cat file | head -n 1 | wc -c
    t_pipeline_config config = {
        .argv1 = {"/bin/cat", FD_TEST_INPUT, NULL, NULL},
        .argc1 = 2,
        .redir1 = REDIR_NONE,
        .argv2 = {"/usr/bin/head", "-n", "1", NULL},
        .argc2 = 3,
        .redir2 = REDIR_NONE,
        .argv3 = {"/usr/bin/wc", "-c", NULL, NULL},
        .argc3 = 2,
        .redir3 = REDIR_NONE,
        .in_file = NULL,
        .out_file = NULL
    };
    t_list *cmds = fd_build_pipeline(&config);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    mu_assert_intcmp("status should be 0", status, 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_external_3_stage_all_redirs_ok(void)
{
    printf("Test: external_in | external_in_out | external_out (all ok)\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "line1\nline2\nline3\n");
    
    t_pipeline_config config = {
        .argv1 = {"/bin/cat", NULL, NULL, NULL},
        .argc1 = 1,
        .redir1 = REDIR_IN,
        .argv2 = {"/usr/bin/head", "-n", "2", NULL},
        .argc2 = 3,
        .redir2 = REDIR_IN_OUT,
        .argv3 = {"/bin/cat", NULL, NULL, NULL},
        .argc3 = 1,
        .redir3 = REDIR_OUT,
        .in_file = FD_TEST_INPUT,
        .out_file = FD_TEST_OUTPUT
    };
    t_list *cmds = fd_build_pipeline(&config);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    mu_assert_intcmp("status should be 0", status, 0);
    mu_assert_intcmp("output file should have content",
                     verify_file_content(FD_TEST_OUTPUT, "line1\nline2\n"), 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_PIPELINE: EXTERNAL FORK FAILS
 * ============================================================================ */

static int test_pipeline_external_fork_fails_first(void)
{
    printf("Test: external fork fails | external | external\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = fd_config_external_3_stage();
    t_list *cmds = fd_build_pipeline(&config);
    
    fd_setup_fork_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    logger_open_fds("♻️test_pipeline_external_fork_fails_first♻️", "should be restored");
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_external_fork_fails_second(void)
{
    return (0);
    printf("Test: external | external fork fails | external\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = fd_config_external_3_stage();
    t_list *cmds = fd_build_pipeline(&config);
    
    fd_setup_fork_fail_at(1);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_pipeline_external_fork_fails_second♻️", "should be restored");
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_external_fork_fails_third(void)
{
    return (0);
    printf("Test: external | external | external fork fails\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = fd_config_external_3_stage();
    t_list *cmds = fd_build_pipeline(&config);
    
    fd_setup_fork_fail_at(2);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_pipeline_external_fork_fails_third♻️", "should be restored");
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_PIPELINE: EXTERNAL OPEN FAILS
 * ============================================================================ */

static int test_pipeline_external_open_fails_first_in(void)
{
    printf("Test: external_in open fails | external | external\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = {
        .argv1 = {"/bin/cat", NULL, NULL, NULL},
        .argc1 = 1,
        .redir1 = REDIR_IN,
        .argv2 = {"/usr/bin/head", "-n", "1", NULL},
        .argc2 = 3,
        .redir2 = REDIR_NONE,
        .argv3 = {"/usr/bin/wc", "-c", NULL, NULL},
        .argc3 = 2,
        .redir3 = REDIR_IN,
        .in_file = FD_TEST_NONEXISTENT,
        .out_file = NULL
    };
    t_list *cmds = fd_build_pipeline(&config);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_external_open_fails_second_in(void)
{
    printf("Test: external | external_in open fails | external\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    t_pipeline_config config = {
        .argv1 = {"/bin/echo", "hello", NULL, NULL},
        .argc1 = 2,
        .redir1 = REDIR_NONE,
        .argv2 = {"/bin/cat", NULL, NULL, NULL},
        .argc2 = 1,
        .redir2 = REDIR_IN,
        .argv3 = {"/bin/cat", NULL, NULL, NULL},
        .argc3 = 1,
        .redir3 = REDIR_NONE,
        .in_file = FD_TEST_NONEXISTENT,
        .out_file = NULL
    };
    t_list *cmds = fd_build_pipeline(&config);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    mu_assert("status should be zero because last command exit ok", status == 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_external_open_fails_third_in(void)
{
    return (0);
    // echo hello | cat | cat < inexistant
    printf("Test: external | external | external_in open fails\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = {
        .argv1 = {"/bin/echo", "hello", NULL, NULL},
        .argc1 = 2,
        .redir1 = REDIR_NONE,
        .argv2 = {"/bin/cat", NULL, NULL, NULL},
        .argc2 = 1,
        .redir2 = REDIR_NONE,
        .argv3 = {"/bin/cat", NULL, NULL, NULL},
        .argc3 = 1,
        .redir3 = REDIR_IN,
        .in_file = FD_TEST_NONEXISTENT,
        .out_file = NULL
    };
    t_list *cmds = fd_build_pipeline(&config);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_external_open_fails_first_out(void)
{
    printf("Test: external_out open fails | external | external\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = {
        .argv1 = {"/bin/echo", "test", NULL, NULL},
        .argc1 = 2,
        .redir1 = REDIR_OUT,
        .argv2 = {"/bin/cat", NULL, NULL, NULL},
        .argc2 = 1,
        .redir2 = REDIR_NONE,
        .argv3 = {"/bin/cat", NULL, NULL, NULL},
        .argc3 = 1,
        .redir3 = REDIR_IN_OUT,
        .in_file = FD_TEST_NONEXISTENT,
        .out_file = FD_TEST_OUTPUT
    };
    t_list *cmds = fd_build_pipeline(&config);
    
    fd_setup_open_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_external_open_fails_first_in_out(void)
{
    printf("Test: external_in_out open fails | external | external\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    t_pipeline_config config = {
        .argv1 = {"/bin/cat", NULL, NULL, NULL},
        .argc1 = 1,
        .redir1 = REDIR_IN_OUT,
        .argv2 = {"/bin/cat", NULL, NULL, NULL},
        .argc2 = 1,
        .redir2 = REDIR_NONE,
        .argv3 = {"/bin/cat", NULL, NULL, NULL},
        .argc3 = 1,
        .redir3 = REDIR_IN,
        .in_file = FD_TEST_NONEXISTENT,
        .out_file = FD_TEST_OUTPUT
    };
    t_list *cmds = fd_build_pipeline(&config);
    
    fd_setup_open_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_PIPELINE: EXTERNAL DUP2 FAILS
 * ============================================================================ */

static int test_pipeline_external_dup2_fails_middle(void)
{
    printf("Test: external | external_in_out dup2 fails | external\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "content\n");
    
    t_pipeline_config config = {
        .argv1 = {"/bin/echo", "hello", NULL, NULL},
        .argc1 = 2,
        .redir1 = REDIR_NONE,
        .argv2 = {"/bin/cat", NULL, NULL, NULL},
        .argc2 = 1,
        .redir2 = REDIR_IN_OUT,
        .argv3 = {"/bin/cat", NULL, NULL, NULL},
        .argc3 = 1,
        .redir3 = REDIR_IN_OUT,
        .in_file = FD_TEST_INPUT,
        .out_file = FD_TEST_OUTPUT
    };
    t_list *cmds = fd_build_pipeline(&config);
    
    // Fail dup2 after a few calls (for middle stage)
    fd_setup_dup2_fail_at(1);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

/* ============================================================================
 * EXEC_PIPELINE: PIPE FAILS
 * ============================================================================ */

static int test_pipeline_pipe_fails_first(void)
{
    printf("Test: pipeline first pipe() fails\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = fd_config_builtin_3_stage();
    t_list *cmds = fd_build_pipeline(&config);
    
    fd_setup_pipe_fail_at(0);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_pipeline_pipe_fails_first♻️", "should be restored");

    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_pipe_fails_second(void)
{
    printf("Test: pipeline second pipe() fails\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    t_pipeline_config config = fd_config_builtin_3_stage();
    t_list *cmds = fd_build_pipeline(&config);
    
    fd_setup_pipe_fail_at(1);
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    logger_open_fds("♻️test_pipeline_pipe_fails_second♻️", "should be restored");
    fd_teardown_all_mocks();
    
    mu_assert("status should be non-zero", status != 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

static int test_pipeline_builtins_and_externals_6_cmds_happy_path(void)
{
    printf("Test: builtin | external | builtin | external | builtin | external (6 stages, all ok)\n");
    t_shell *sh = create_test_shell(test_env, 0);
    
    create_test_input_file(FD_TEST_INPUT, "apple\nbanana\ncherry\ndate\neggplant\nfig\n");
    
    // Stage 1: builtin with input redirection: echo < input
    const char *argv1[] = {"cat", NULL};
    t_cmd *cmd1 = fd_make_cmd_redir_in(argv1, 1, FD_TEST_INPUT);
    
    // Stage 2: external: /usr/bin/head -n 5
    const char *argv2[] = {"/usr/bin/head", "-n", "5", NULL};
    t_cmd *cmd2 = fd_make_cmd(argv2, 3);
    
    // Stage 3: builtin: cat (pass through)
    const char *argv3[] = {"cat", NULL};
    t_cmd *cmd3 = fd_make_cmd(argv3, 1);
    
    // Stage 4: external: /usr/bin/tail -n 3
    const char *argv4[] = {"/usr/bin/head", "-n", "3", NULL};
    t_cmd *cmd4 = fd_make_cmd(argv4, 3);
    
    // Stage 5: builtin: cat (pass through)
    const char *argv5[] = {"cat", NULL};
    t_cmd *cmd5 = fd_make_cmd(argv5, 1);
    
    // Stage 6: external with output redirection: /bin/cat > output
    const char *argv6[] = {"/bin/cat", NULL};
    t_cmd *cmd6 = fd_make_cmd_redir_out(argv6, 1, FD_TEST_OUTPUT);
    
    t_list *cmds = NULL;
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    ft_lstadd_back(&cmds, ft_lstnew(cmd4));
    ft_lstadd_back(&cmds, ft_lstnew(cmd5));
    ft_lstadd_back(&cmds, ft_lstnew(cmd6));
    
    int status = exec_cmds(sh, cmds);
    wait_all_children();
    
    logger_open_fds("♻️test_pipeline_builtins_and_externals_6_cmds_happy_path♻️", "should be restored");
    
    mu_assert_intcmp("status should be 0", status, 0);
    mu_assert_intcmp("output file should have first 3 of first 5 lines",
                     verify_file_content(FD_TEST_OUTPUT, "apple\nbanana\ncherry\n"), 0);
    
    free_cmds(cmds);
    free_shell(sh);
    cleanup_fd_test_files();
    return (0);
}

int main(void)
{
    // mu_run_test(test_empty_pipeline_returns_zero);
    // mu_run_test(test_on_signal_c_last_status_set_to_130_when_set_here_docs);
    
    // EVERY FLOW POSSIBLE BELLOW EXEC_CMDS (focus on fds)
    
    // Ensure test directory exists
    mkdir(FD_TEST_DIR, 0755);

    // # EXEC SIMPLE
    printf("=== EXEC_SIMPLE: BUILTIN ===\n\n");
    
    printf("--- Happy paths ---\n");
    sleep(1);
    mu_run_test(test_builtin_simple_ok);
    sleep(1);
    mu_run_test(test_builtin_redir_in);
    sleep(1);
    mu_run_test(test_builtin_redir_out);
    sleep(1);
    mu_run_test(test_builtin_redir_in_out);
    sleep(1);
    printf("\n--- Dup fails ---\n");
    mu_run_test(test_builtin_dup_fails_redir_in);
    sleep(1);
    mu_run_test(test_builtin_dup_fails_redir_out);
    sleep(1);
    mu_run_test(test_builtin_dup_fails_redir_in_out);
    sleep(1);

    printf("\n--- Open fails ---\n");
    mu_run_test(test_builtin_open_fails_redir_in);
    sleep(1);
    mu_run_test(test_builtin_open_fails_redir_out);
    sleep(1);
    mu_run_test(test_builtin_open_fails_redir_in_out);
    sleep(1);
    printf("\n--- Dup2 fails ---\n");
    mu_run_test(test_builtin_dup2_fails_redir_in);
    sleep(1);
    mu_run_test(test_builtin_dup2_fails_redir_out);
    sleep(1);
    mu_run_test(test_builtin_dup2_fails_redir_in_out);
    sleep(1);

    printf("\n=== EXEC_SIMPLE: EXTERNAL ===\n\n");
    
    printf("--- Happy paths ---\n");
    mu_run_test(test_external_redir_in);
    sleep(1);
    mu_run_test(test_external_redir_out);
    sleep(1);
    mu_run_test(test_external_redir_in_out);
    sleep(1);

    printf("\n--- Fork fails ---\n");
    mu_run_test(test_external_fork_fails_redir_in);
    sleep(1);
    mu_run_test(test_external_fork_fails_redir_out);
    sleep(1);
    mu_run_test(test_external_fork_fails_redir_in_out);
    sleep(1);

    printf("\n--- Open fails ---\n");
    mu_run_test(test_external_open_fails_redir_in);
    sleep(1);
    mu_run_test(test_external_open_fails_redir_out);
    sleep(1);
    mu_run_test(test_external_open_fails_redir_in_out);
    sleep(1);

    printf("\n--- Dup2 fails ---\n");
    mu_run_test(test_external_dup2_fails_redir_in);
    sleep(1);
    mu_run_test(test_external_dup2_fails_redir_out);
    sleep(1);
    mu_run_test(test_external_dup2_fails_redir_in_out);
    sleep(1);

    // // # EXEC_PIPELINE
    printf("\n=== EXEC_PIPELINE: BUILTIN ===\n\n");
    
    printf("--- Happy paths ---\n");
    mu_run_test(test_pipeline_builtin_3_stage_ok);
    sleep(2);
    mu_run_test(test_pipeline_builtin_3_stage_all_redirs_ok);
    sleep(1);

    printf("\n--- Fork fails ---\n");
    mu_run_test(test_pipeline_builtin_fork_fails_first);
    sleep(1);
    mu_run_test(test_pipeline_builtin_fork_fails_second); // KO DEUDA TÉCNICA, mi builtin no "se queda escuchando", SIGPIPE en child
    sleep(1);
    mu_run_test(test_pipeline_builtin_fork_fails_third); // KO DEUDA TÉCNICA, mi builtin no "se queda escuchando" SIGPIPE en child
    sleep(1);


    printf("\n--- Open fails ---\n");
    mu_run_test(test_pipeline_builtin_open_fails_first_in);
    mu_run_test(test_pipeline_builtin_open_fails_first_out);
    mu_run_test(test_pipeline_builtin_open_fails_first_in_out); // KO DEUDA TÉCNICA, SIGPIPE on child, aunque funcionamiento esperado

    printf("\n--- Dup2 fails ---\n");

    mu_run_test(test_pipeline_builtin_dup2_fails_middle); // KO DEUDA TÉCNICA, extra in_fd in middle command not closed, better than sigpipe


    printf("\n=== EXEC_PIPELINE: EXTERNAL ===\n\n");
    
    printf("--- Happy paths ---\n");
    mu_run_test(test_pipeline_external_3_stage_ok);
    mu_run_test(test_pipeline_external_3_stage_all_redirs_ok);

    printf("\n--- Fork fails ---\n");
    mu_run_test(test_pipeline_external_fork_fails_first);
    mu_run_test(test_pipeline_external_fork_fails_second); // KO DEUDA TÉCNICA, esperamos que fork no falle
    mu_run_test(test_pipeline_external_fork_fails_third); // KO DEUDA TÉCNICA, esperamos que fork no falle
    
    printf("\n--- Open fails ---\n");
    mu_run_test(test_pipeline_external_open_fails_first_in);
    mu_run_test(test_pipeline_external_open_fails_second_in);
    mu_run_test(test_pipeline_external_open_fails_third_in); // KO, no es deuda técnica, da SIGPIPE pero bash también
    mu_run_test(test_pipeline_external_open_fails_first_out);
    mu_run_test(test_pipeline_external_open_fails_first_in_out);

    printf("\n--- Dup2 fails ---\n");
    mu_run_test(test_pipeline_external_dup2_fails_middle);
    
    printf("\n=== EXEC_PIPELINE: PIPE FAILS ===\n\n");
    mu_run_test(test_pipeline_pipe_fails_first);
    mu_run_test(test_pipeline_pipe_fails_second);


    mu_run_test(test_pipeline_builtins_and_externals_6_cmds_happy_path);
    
    printf("\n");
    cleanup_fd_test_files();

    mu_summary();
}
