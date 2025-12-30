#include <stdlib.h>
#include <string.h>
#include "minunit.h"
#include "../common/test_helpers.h"

#include "../../include/exec.h"
#include "../../include/minishell.h"
#include "../../Libft/include/libft.h"

// BRAINSTORMING

// Here we make use of indirection via syswrap (not just injection via env variables)
// Brainstorming:
// Qué pasa con un here doc en el que solo se pasa el delimiter?
// Debo comerme el \n final?
// Qué pasa con las líneas en blanco? las incluyo o me las salto?
// Heredoc buffering: prefer fetch_here_doc_from_user(...) returning the full content; then set_here_doc(...) writes that to a temp file and replaces the command’s heredoc redir with R_IN pointing to that temp path. 
// heredoc expansion is included in the subject
/*
Given this test:
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


/*
typedef enum e_redir_type {
	R_IN,          // <
	R_OUT_TRUNC,   // >
	R_OUT_APPEND,  // >>
	R_HEREDOC      // <<
}   t_redir_type;

typedef struct s_redir {
	t_redir_type      type;
	int               fd;
	char             *target; // path or delimiter for here doc
	int               quoted;
}   t_redir;

typedef struct s_cmd {
	char           **argv;
	t_list         *redirs;
}   t_cmd;
*/

// Test: can I have here docs in the middle of it, I mean does it work if my here doc is in a command mid pipeline?? It should

// Test (I don't know), should fetch here_doc happen in a subprocess? how does it work in bash? 

// Manual test also with valgrind for memory leaks (readline has some leaks)

// Test expand variables, should expand variables in here docs (until next space or until next '$' ????? )

// What is considered a variable? $USER, ${USER}, $HOME, ${HOME}, etc. what should I do if someone enteres $HOM? (instead of HOME)


// TESTS

// Mock env for tests
const char *test_env[] = {
    "USER=saalarco",
    "HOME=/home/saalarco",
    "SHELL=/bin/bash",
    NULL
};

// TODO: make this reusable by multiple tests
static const char *heredoc_data_test2 = "line1\nline2";
char *readline_wrap_test2(const char *prompt)
{
    static size_t call_count = 0;
    char *line = NULL;

    (void)prompt; // unused

    if (call_count == 0) {
        line = strdup("line1");
    } else if (call_count == 1) {
        line = strdup("line2");
    } else if (call_count == 2) {
        line = strdup("EOF");
    } else {
        line = NULL; // Simulate EOF
    }

    call_count++;
    return line;
}

/*
** set_here_doc.c
*/

// 1. Test on various cmds with no R_HEREDOC redir, target doesn't change
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
    syswrap_set_readline(readline_wrap_test2);

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
    t_redir *redir1 = make_redir(R_HEREDOC, delimiter, 0, 0);
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
    mu_assert_strcmp("target has changed when here_doc involved", redir_1_after_set_here_doc->target, delimiter);
    mu_assert("fd has changed when here_doc involved", 1 == 1);

    int fd = redir_1_after_set_here_doc->fd;
    mu_assert("fd not null", fd != NULL);
    char *heredoc_data = "line1\nline2\n";
    char buffer[1024];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    buffer[bytes_read] = '\0';
    mu_assert_strcmp("here_doc content mismatch", buffer, heredoc_data_test2);

    free_cmds(pipe_head);
    free_shell(sh);

    /* restore original readline */
    syswrap_set_readline(NULL);

    return 0;
}

int main(void)
{
    mu_run_test(test_various_cmds_no_here_doc_unchanges_returns_0);
    mu_summary();
}