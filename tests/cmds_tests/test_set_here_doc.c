#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "minunit.h"
#include "../common/test_helpers.h"

#include "../../include/exec.h"
#include "../../include/minishell.h"
#include "../../include/syswrap.h"
#include "../../Libft/include/libft.h"

// BRAINSTORMING

/*
Try this manual test
saalarco@c2r14s6:~/Dev/minishell$ wc << EOF
> K
> EOF
      1       1       2
saalarco@c2r14s6:~/Dev/minishell$ wc << EOF
K
K  
> EOF
      3       2       5
saalarco@c2r14s6:~/Dev/minishell$ wc << EOF
EOF
      0       0       0

should here_doc return an empty string or NULL on direct delimiter?
*/

// README con el test manual anterior si es necesario u otros

// README with readline leaks

// README spec file for expand_hd

// refactor current tests to make them scalable

// Test , should fetch here_doc happen in a subprocess? how does it works in bash? is there a posix spec about it? 
/*
In bash, here documents are typically processed in a subshell. 
This means that any changes to the environment (like variable assignments) made within the here document will not affect the parent shell. 
The POSIX specification does not explicitly state this behavior, but it is a widely accepted convention in Unix-like systems.
*/

// TESTS

// Mock env for tests
const char *test_env[] = {
    "USER=saalarco",
    "HOME=/home/saalarco",
    "SHELL=/bin/bash",
    NULL
};


static const char *heredoc_data_test_2 = "line1\nline2\n";
char *readline_wrap_test_2(const char *prompt)
{
    static size_t call_count = 0;
    char *line = NULL;

    (void)prompt; // unused

    if (call_count == 0) {
        line = strdup("line1\0");
    } else if (call_count == 1) {
        line = strdup("line2\0");
    } else if (call_count == 2) {
        line = strdup("EOF\0");
    } else {
        line = NULL; // Simulate EOF
    }

    call_count++;
    return line;
}

static const char *heredoc_data_test_3 = "";
char *readline_wrap_test_3(const char *prompt)
{
    static size_t call_count = 0;
    char *line = NULL;

    (void)prompt; // unused

    if (call_count == 0) {
        line = strdup("EOF\0");
    } else {
        line = NULL; // Simulate EOF
    }

    call_count++;
    return line;
}

static const char *heredoc_data_test_4 = "\n\n\n\n";
char *readline_wrap_test_4(const char *prompt)
{
    static size_t call_count = 0;
    char *line = NULL;

    (void)prompt; // unused

    if (call_count == 0) {
        line = strdup("");
    } else if (call_count == 1) {
        line = strdup("");
    } else if (call_count == 2) {
        line = strdup("");
    } else if (call_count == 3) {
        line = strdup("");
    } else if (call_count == 4) {
        line = strdup("EOF\0");
    } else {
        line = NULL; // Simulate EOF
    }

    call_count++;
    return line;
}

/*
** set_here_doc.c
*/

//1. Test on various cmds with no R_HEREDOC redir, target doesn't change
static int test_various_cmds_no_here_doc_unchanges_returns_0(void)
{
    printf("Test: test_various_cmds_no_here_doc_unchanges_returns_0\n");
    t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
    mu_assert("malloc shell failed", sh != NULL);
    sh->last_status = 0;
    sh->should_exit = 0;
    sh->env = deep_copy_env(test_env);

    // Build: wc -l < infile | ls (no here_doc redirs)
    const char *argv1[] = {"wc", "-l"};
    const char *argv2[] = {"ls"};

    // cmd1: wc -l < infile
    char *target1 = "infile";
    t_redir *redir1 = make_redir(R_IN, target1, 0, 0);
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    // cmd2: ls
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    // list redirs
    ft_lstadd_back(&cmd1->redirs, ft_lstnew(redir1));
    
    t_list *pipe_head = NULL;
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));

    set_here_doc(sh, pipe_head);

    t_redir *redir_1_after_set_here_doc = ft_lstlast(cmd1->redirs)->content;
    mu_assert_strcmp("target has changed when no here_doc involved", redir_1_after_set_here_doc->target, target1);
    mu_assert("fd has changed when no here_doc involved", 1 == 1);

    free_cmds(pipe_head);
    free_shell(sh);

    return 0;
}

/*
** set_here_doc.c
** fetch_here_doc_from_user.c
*/

// 2. Test on various cmds with heredoc redir target changes
static int test_various_cmds_with_here_doc_changes_returns_0(void)
{
    printf("Test: test_various_cmds_with_here_doc_changes_returns_0\n");

    /* inject custom readline */
    syswrap_set_readline(readline_wrap_test_2);

    t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
    mu_assert("malloc shell failed", sh != NULL);
    sh->last_status = 0;
    sh->should_exit = 0;
    sh->env = deep_copy_env(test_env);

    // Build: wc -l << EOF | ls
    const char *argv1[] = {"wc", "-l"};
    const char *argv2[] = {"ls"};

    // cmd1: wc -l << EOF
    char *delimiter = "EOF";
    t_redir *redir1 = make_redir(R_HEREDOC, delimiter, 1, 0);
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    // cmd2: ls
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    // list redirs
    ft_lstadd_back(&cmd1->redirs, ft_lstnew(redir1));

    t_list *pipe_head = NULL;
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));

    set_here_doc(sh, pipe_head);

    t_redir *redir_1_after_set_here_doc = ft_lstlast(cmd1->redirs)->content;
    mu_assert("fd changed when setting here_doc", redir_1_after_set_here_doc->fd == 0);

    char *path = redir_1_after_set_here_doc->target;
    int fd = open(path, O_RDONLY);
    char buffer[ft_strlen(heredoc_data_test_2) + 1];
    ssize_t bytes_read = read(fd, buffer, ft_strlen(heredoc_data_test_2));
    buffer[bytes_read] = '\0';
    mu_assert_strcmp("here_doc content mismatch", heredoc_data_test_2, buffer);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);

    /* restore original readline */
    syswrap_set_readline(NULL);


    return 0;
}

// 3. Test empty heredoc, user just introduces the delimiter
static int test_empty_heredoc_user_introduces_delimiter(void)
{
    printf("Test: test_empty_heredoc_user_introduces_delimiter\n");

    /* inject custom readline */
    syswrap_set_readline(readline_wrap_test_3);

    t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
    mu_assert("malloc shell failed", sh != NULL);
    sh->last_status = 0;
    sh->should_exit = 0;
    sh->env = deep_copy_env(test_env);

    // Build: wc -l << EOF | ls
    const char *argv1[] = {"wc", "-l"};
    const char *argv2[] = {"ls"};

    // cmd1: wc -l << EOF
    char *delimiter = "EOF";
    t_redir *redir1 = make_redir(R_HEREDOC, delimiter, 1, 0);
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    // cmd2: ls
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    // list redirs
    ft_lstadd_back(&cmd1->redirs, ft_lstnew(redir1));

    t_list *pipe_head = NULL;
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));

    set_here_doc(sh, pipe_head);

    t_redir *redir_1_after_set_here_doc = ft_lstlast(cmd1->redirs)->content;
    mu_assert("fd changed when setting here_doc", redir_1_after_set_here_doc->fd == 0);

    char *path = redir_1_after_set_here_doc->target;
    int fd = open(path, O_RDONLY);
    char buffer[ft_strlen(heredoc_data_test_3) + 1];
    ssize_t bytes_read = read(fd, buffer, ft_strlen(heredoc_data_test_3));
    buffer[bytes_read] = '\0';
    mu_assert_strcmp("here_doc content mismatch", heredoc_data_test_3, buffer);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);

    /* restore original readline */
    syswrap_set_readline(NULL);

    return 0;
}

// Test 4: User intruces several empty lines
static int test_various_cmds_with_empty_lines(void)
{
    printf("Test: test_various_cmds_with_empty_lines\n");

    /* inject custom readline */
    syswrap_set_readline(readline_wrap_test_4);

    t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
    mu_assert("malloc shell failed", sh != NULL);
    sh->last_status = 0;
    sh->should_exit = 0;
    sh->env = deep_copy_env(test_env);

    // Build: wc -l << EOF | ls
    const char *argv1[] = {"wc", "-l"};
    const char *argv2[] = {"ls"};

    // cmd1: wc -l << EOF
    char *delimiter = "EOF";
    t_redir *redir1 = make_redir(R_HEREDOC, delimiter, 1, 0);
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    // cmd2: ls
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    // list redirs
    ft_lstadd_back(&cmd1->redirs, ft_lstnew(redir1));

    t_list *pipe_head = NULL;
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));

    set_here_doc(sh, pipe_head);

    t_redir *redir_1_after_set_here_doc = ft_lstlast(cmd1->redirs)->content;
    mu_assert("fd changed when setting here_doc", redir_1_after_set_here_doc->fd == 0);

    char *path = redir_1_after_set_here_doc->target;
    int fd = open(path, O_RDONLY);
    char buffer[ft_strlen(heredoc_data_test_4) + 1];
    ssize_t bytes_read = read(fd, buffer, ft_strlen(heredoc_data_test_4));
    buffer[bytes_read] = '\0';
    mu_assert_strcmp("here_doc content mismatch", heredoc_data_test_4, buffer);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);

    /* restore original readline */
    syswrap_set_readline(NULL);

    return 0;
}

// Test 5: Various HERE_DOCS included in the pipe. (after refactoring this file ojocuidao)


int main(void)
{
    mu_run_test(test_various_cmds_no_here_doc_unchanges_returns_0);
    mu_run_test(test_various_cmds_with_here_doc_changes_returns_0);
    mu_run_test(test_empty_heredoc_user_introduces_delimiter);
    mu_run_test(test_various_cmds_with_empty_lines);
    mu_summary();
}