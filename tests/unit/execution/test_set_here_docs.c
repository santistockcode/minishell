 #include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

#include "../../../include/exec.h"
#include "../../../include/minishell.h"
#include "../../../include/syswrap.h"
#include "../../../Libft/include/libft.h"

volatile sig_atomic_t g_exit_status = 0;

// TESTS

// Mock env for tests
const char *test_env[] = {
    "USER=saalarco",
    "HOME=/home/saalarco",
    "SHELL=/bin/bash",
    NULL
};

// ============================================================================
// Generic readline mock generator
// ============================================================================

typedef struct s_readline_mock {
    const char **lines;
    size_t call_count;
    size_t fail_at;
} t_readline_mock;

static t_readline_mock g_mock = {NULL, 0, 0};

static char *generic_readline_mock(const char *prompt)
{
    (void)prompt;
    
    if (!g_mock.lines || !g_mock.lines[g_mock.call_count])
        return NULL;
    
    char *line = strdup(g_mock.lines[g_mock.call_count]);
    g_mock.call_count++;
    return line;
}

static char *generic_readline_mock_fails(const char *prompt)
{
    (void)prompt;
    if (g_mock.call_count == g_mock.fail_at)
    {
        errno = ENOMEM;
        return NULL;
    }
    if (!g_mock.lines || !g_mock.lines[g_mock.call_count])
        return NULL;
    char *line = strdup(g_mock.lines[g_mock.call_count]);
    g_mock.call_count++;
    return line;
}

static void setup_readline_mock(const char **lines)
{
    g_mock.lines = lines;
    g_mock.call_count = 0;
    syswrap_set_readline(generic_readline_mock);
}

static void setup_readline_to_fail_at(const char **lines, int fail_at)
{
    g_mock.lines = lines;
    g_mock.call_count = 0;
    g_mock.fail_at = fail_at;
    syswrap_set_readline(generic_readline_mock_fails);
}

static void teardown_readline_mock(void)
{
    syswrap_set_readline(NULL);
    g_mock.lines = NULL;
    g_mock.call_count = 0;
}

// ============================================================================
// Generic open mock and utils
// ============================================================================

typedef struct s_open_mock {
    int (*open_func)(const char *, int, int);
    int call_count;
    int fail_at;
} t_open_mock;

static t_open_mock g_open_mock = {NULL, 0, 0};

int open_wrap_eaccess(const char *path, int oflag, int mode) 
{
    if (g_open_mock.call_count == g_open_mock.fail_at)
    {
        errno = EACCES;
        return -1;
    }
    g_open_mock.call_count++;
    return open(path, oflag, mode);
}

static void setup_open_fails_at_call(int (*open_func)(const char *, int, int), int fail_at)
{
    g_open_mock.call_count = 0;
    g_open_mock.fail_at = fail_at;
    syswrap_set_open((t_open_fn)open_func);
}

// ============================================================================
// Generic read mock generator
// ============================================================================

typedef struct s_read_mock {
    const char **lines;
} t_read_mock;

static size_t generic_read_mock(int fd, void *buf, size_t count)
{
    (void)fd;
    (void)buf;
    (void)count;

    errno = EACCES;
    return (0);
}

static void setup_read_fails(size_t (*read_func)(int, void *, size_t))
{
    syswrap_set_read((t_read_fn)read_func);
}

// ============================================================================
// Test helpers for test_here_doc
// ============================================================================


static int verify_heredoc_content(const char *path, const char *expected)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;
    
    size_t expected_len = strlen(expected);
    char *buffer = malloc(expected_len + 1);
    if (!buffer) {
        close(fd);
        return -1;
    }
    
    ssize_t bytes_read = read(fd, buffer, expected_len);
    buffer[bytes_read] = '\0';
    close(fd);
    
    int match = strcmp(buffer, expected) == 0;
    free(buffer);
    
    return match ? 0 : -1;
}

static t_list *build_generic_pipeline_with_heredoc_first_command(const char *delimiter, int quoted)
{
    const char *argv1[] = {"wc"};
    const char *argv2[] = {"ls"};
    
    // cmd1: wc -l << EOF
    t_redir *redir1 = make_redir(R_HEREDOC, (char*)delimiter, quoted, 0);
    t_cmd *cmd1 = new_cmd_from_args(argv1, 1);
    ft_lstadd_back(&cmd1->redirs, ft_lstnew(redir1));
    
    // cmd2: ls
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    
    t_list *pipe_head = NULL;
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));
    
    return pipe_head;
}

static t_list *build_generic_pipeline_with_heredoc_second_command(const char *delimiter, int quoted)
{
    const char *argv1[] = {"wc"};
    const char *argv2[] = {"ls"};

    // cmd1: wc
    t_cmd *cmd1 = new_cmd_from_args(argv1, 1);
    
    // cmd2: ls << EOF
    t_redir *redir1 = make_redir(R_HEREDOC, (char*)delimiter, quoted, 0);
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmd2->redirs, ft_lstnew(redir1));

    t_list *pipe_head = NULL;
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));

    return pipe_head;
}

static t_list *build_pipeline_no_heredoc(void)
{
    const char *argv1[] = {"wc", "-l"};
    const char *argv2[] = {"ls"};
    
    // cmd1: wc -l < infile
    t_redir *redir1 = make_redir(R_IN, "infile", 0, 0);
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmd1->redirs, ft_lstnew(redir1));
    
    // cmd2: ls
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    
    t_list *pipe_head = NULL;
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));
    
    return pipe_head;
}

// builds generic pipeline with 6 commands and 3 here_docs
static t_list *build_generic_pipeline_various_heredocs(const char *delimiter1, int quoted1,
    const char *delimiter2, int quoted2, const char *delimiter3, int quoted3)
{
    const char *argv1[] = {"wc", "-l"}; // HEREDOC
    const char *argv2[] = {"ls"}; // HEREDOC
    const char *argv3[] = {"grep", "test"};
    const char *argv4[] = {"sort"}; // HEREDOC
    const char *argv5[] = {"uniq"};

    // cmd1: wc -l << EOF
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    t_redir *redir1 = make_redir(R_HEREDOC, (char*)delimiter1, quoted1, 0);
    ft_lstadd_back(&cmd1->redirs, ft_lstnew(redir1));

    // cmd2: ls << EOF
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    t_redir *redir2 = make_redir(R_HEREDOC, (char*)delimiter2, quoted2, 0);
    ft_lstadd_back(&cmd2->redirs, ft_lstnew(redir2));

    // cmd3: grep test
    t_cmd *cmd3 = new_cmd_from_args(argv3, 2);

    // cmd4: sort << EOF
    t_cmd *cmd4 = new_cmd_from_args(argv4, 1);
    t_redir *redir4 = make_redir(R_HEREDOC, (char*)delimiter3, quoted3, 0);
    ft_lstadd_back(&cmd4->redirs, ft_lstnew(redir4));

    // cmd5: uniq
    t_cmd *cmd5 = new_cmd_from_args(argv5, 1);

    t_list *pipe_head = NULL;
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd3));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd4));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd5));

    return pipe_head;
}


// ============================================================================
// TESTS
// ============================================================================

// Test 1: Commands without heredoc remain unchanged
static int test_various_cmds_no_here_doc_unchanges_returns_0(void)
{
    printf("Test: test_various_cmds_no_here_doc_unchanges_returns_0\n");

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);
    
    t_list *pipe_head = build_pipeline_no_heredoc();
    const char *original_target = "infile";
    
    set_here_docs(sh, pipe_head);
    
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir_after = ft_lstlast(cmd1->redirs)->content;
    
    mu_assert_strcmp("target changed when no heredoc involved", 
                     redir_after->target, original_target);
    
    free_cmds(pipe_head);
    free_shell(sh);
    
    return 0;
}

// Test 2: Heredoc on first command, multiple lines, no expansion
static int test_heredoc_first_command_multiple_lines_no_expansion(void)
{
    printf("Test: test_heredoc_first_command_multiple_lines_no_expansion\n");
    
    const char *user_input[] = {"line1", "$HOME", "EOF", NULL};
    const char *expected_content = "line1\n$HOME\n";
    
    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);
    
    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 1);
    
    set_here_docs(sh, pipe_head);
    
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir_after = ft_lstlast(cmd1->redirs)->content;
    
    mu_assert("fd changed when setting heredoc", redir_after->fd == 0);
    mu_assert("heredoc content mismatch", 
              verify_heredoc_content(redir_after->target, expected_content) == 0);
    
    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    
    return 0;
}


// Test 3: Heredoc on first command, multiple lines, with expansion
static int test_heredoc_first_command_multiple_lines_with_expansion(void)
{
    printf("Test: test_heredoc_first_command_multiple_lines_with_expansion\n");

    const char *user_input[] = {"line1", "I'm $HOME", "EOF", NULL};
    const char *expected_content = "line1\nI'm /home/saalarco\n";

    setup_readline_mock(user_input);
    
    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);
    
    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);
    
    set_here_docs(sh, pipe_head);
    
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir_after = ft_lstlast(cmd1->redirs)->content;
    
    mu_assert("fd changed when setting heredoc", redir_after->fd == 0);
    mu_assert("heredoc content mismatch", 
              verify_heredoc_content(redir_after->target, expected_content) == 0);
    
    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    
    return 0;
}

// Test 4: Heredoc on first command, 4 lines, with expansion on last one
static int test_various_cmds_with_heredoc_changes_returns_0(void)
{
    printf("Test: test_various_cmds_with_heredoc_changes_returns_0\n");

    const char *user_input[] = {"line1", "line2", "line3", "$HOME", "EOF", NULL};
    const char *expected_content = "line1\nline2\nline3\n/home/saalarco\n";

    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);
    
    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);
    
    set_here_docs(sh, pipe_head);
    
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir_after = ft_lstlast(cmd1->redirs)->content;
    
    mu_assert("fd changed when setting heredoc", redir_after->fd == 0);
    mu_assert("heredoc content mismatch", 
              verify_heredoc_content(redir_after->target, expected_content) == 0);
    
    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    
    return 0;
}

// Test 5: Empty heredoc (delimiter immediately), no expansion
static int test_empty_heredoc_user_introduces_delimiter(void)
{
    printf("Test: test_empty_heredoc_user_introduces_delimiter\n");
    
    const char *user_input[] = {"EOF", NULL};
    const char *expected_content = "";
    
    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);
    
    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 1);
    
    set_here_docs(sh, pipe_head);
    
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir_after = ft_lstlast(cmd1->redirs)->content;
    
    mu_assert("fd changed when setting heredoc", redir_after->fd == 0);
    mu_assert("heredoc content mismatch", 
              verify_heredoc_content(redir_after->target, expected_content) == 0);
    
    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    
    return 0;
}

// Test 6: Empty heredoc (delimiter immediately), with expansion
static int test_empty_heredoc_user_introduces_delimiter_with_expansion(void)
{
    printf("Test: test_empty_heredoc_user_introduces_delimiter_with_expansion\n");

    const char *user_input[] = {"EOF", NULL};
    const char *expected_content = "";
    
    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);
    
    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);
    
    set_here_docs(sh, pipe_head);
    
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir_after = ft_lstlast(cmd1->redirs)->content;
    
    mu_assert("fd changed when setting heredoc", redir_after->fd == 0);
    mu_assert("heredoc content mismatch", 
              verify_heredoc_content(redir_after->target, expected_content) == 0);
    
    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    
    return 0;
}

// Test 7: Heredoc with multiple empty lines
static int test_various_cmds_with_empty_lines(void)
{
    printf("Test: test_various_cmds_with_empty_lines\n");
    
    const char *user_input[] = {"", "", "", "", "EOF", NULL};
    const char *expected_content = "\n\n\n\n";
    
    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);
    
    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 1);
    
    set_here_docs(sh, pipe_head);
    
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir_after = ft_lstlast(cmd1->redirs)->content;
    
    mu_assert("fd changed when setting heredoc", redir_after->fd == 0);
    mu_assert("heredoc content mismatch", 
              verify_heredoc_content(redir_after->target, expected_content) == 0);
    
    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    
    return 0;
}

// Test 8: Heredoc on second command, multiple lines, no expansion
static int test_heredoc_second_command_multiple_lines_no_expansion(void)
{
    printf("Test: test_heredoc_second_command_multiple_lines_no_expansion\n");

    const char *user_input[] = {"line 1", "$HOME", "EOF", NULL};
    const char *expected_content = "line 1\n$HOME\n";

    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_second_command("EOF", 1);

    set_here_docs(sh, pipe_head);

    t_cmd *cmd2 = (t_cmd*)ft_lstlast(pipe_head)->content;
    t_redir *redir_after = ft_lstlast(cmd2->redirs)->content;

    mu_assert("fd changed when setting heredoc", redir_after->fd == 0);
    mu_assert("heredoc content mismatch",
              verify_heredoc_content(redir_after->target, expected_content) == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();

    return 0;
}

// Test 9: Heredoc on second command, multiple lines, expanded
static int test_heredoc_second_command_multiple_lines_with_expansion(void)
{
    printf("Test: test_heredoc_second_command_multiple_lines_with_expansion\n");

    const char *user_input[] = {"line 1", "$USER", "EOF", NULL};
    const char *expected_content = "line 1\nsaalarco\n";

    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_second_command("EOF", 0);

    set_here_docs(sh, pipe_head);

    t_cmd *cmd2 = (t_cmd*)ft_lstlast(pipe_head)->content;
    t_redir *redir_after = ft_lstlast(cmd2->redirs)->content;

    mu_assert("fd changed when setting heredoc", redir_after->fd == 0);
    mu_assert("heredoc content mismatch",
              verify_heredoc_content(redir_after->target, expected_content) == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();

    return 0;
}

// Test 10: 3 heredocs, multiple lines, no expansion
static int test_heredoc_multiple_commands_multiple_lines_no_expansion(void)
{
    printf("Test: test_heredoc_multiple_commands_multiple_lines_no_expansion\n");

    const char *user_input[] = {
        "line1", "line2", "EOF",
        "line1", "line2", "EOF",
        "line1", "line2", "EOF",
        NULL
    };
    const char *expected_content = "line1\nline2\n";

    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    // segmentation fault when returning
    t_list *pipe_head = build_generic_pipeline_various_heredocs("EOF", 1, "EOF", 1, "EOF", 1);

    set_here_docs(sh, pipe_head);

    // verify path vs expected for every command
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir1_after = ft_lstlast(cmd1->redirs)->content;
    mu_assert("heredoc content mismatch cmd1", verify_heredoc_content(redir1_after->target, expected_content) == 0);

    t_cmd *cmd2 = (t_cmd*)pipe_head->next->content;
    t_redir *redir2_after = ft_lstlast(cmd2->redirs)->content;
    mu_assert("heredoc content mismatch cmd2", verify_heredoc_content(redir2_after->target, expected_content) == 0);

    t_cmd *cmd4 = (t_cmd*)pipe_head->next->next->next->content;
    t_redir *redir4_after = ft_lstlast(cmd4->redirs)->content;
    mu_assert("heredoc content mismatch cmd4", verify_heredoc_content(redir4_after->target, expected_content) == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();

    return 0;
}

// Test 11: 3 heredocs, multiple lines, expanded
static int test_heredoc_multiple_commands_multiple_lines_with_expansion(void)
{
    printf("Test: test_heredoc_multiple_commands_multiple_lines_with_expansion\n");

    const char *user_input[] = {
        "line1", "$USER", "EOF",
        "line1", "$USER", "EOF",
        "line1", "$USER", "EOF",
        NULL
    };
    const char *expected_content = "line1\nsaalarco\n";

    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_various_heredocs("EOF", 0, "EOF", 0, "EOF", 0);

    set_here_docs(sh, pipe_head);

    // verify path vs expected for every command
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir1_after = ft_lstlast(cmd1->redirs)->content;
    mu_assert("heredoc content mismatch cmd1", verify_heredoc_content(redir1_after->target, expected_content) == 0);

    t_cmd *cmd2 = (t_cmd*)pipe_head->next->content;
    t_redir *redir2_after = ft_lstlast(cmd2->redirs)->content;
    mu_assert("heredoc content mismatch cmd2", verify_heredoc_content(redir2_after->target, expected_content) == 0);

    t_cmd *cmd4 = (t_cmd*)pipe_head->next->next->next->content;
    t_redir *redir4_after = ft_lstlast(cmd4->redirs)->content;
    mu_assert("heredoc content mismatch cmd4", verify_heredoc_content(redir4_after->target, expected_content) == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();

    return 0;
}

// Test 12: test that last status code is expanded correctly to 0
static int test_last_status_expanded_to_0(void)
{
    printf("Test: test_last_status_expanded_to_0\n");

    const char *user_input[] = {
        "line1", "$?", "EOF",
        NULL
    };
    const char *expected_content = "line1\n0\n";

    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);

    set_here_docs(sh, pipe_head);

    // verify path vs expected for every command
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir1_after = ft_lstlast(cmd1->redirs)->content;
    mu_assert("heredoc content mismatch cmd1", verify_heredoc_content(redir1_after->target, expected_content) == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();

    return 0;
}

// Test 13: test that last status code is expanded correctly to 127
static int test_last_status_expanded_to_127(void)
{
    printf("Test: test_last_status_expanded_to_127\n");

    const char *user_input[] = {
        "line1", "$?", "EOF",
        NULL
    };
    const char *expected_content = "line1\n127\n";

    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 127);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);

    set_here_docs(sh, pipe_head);

    // verify path vs expected for every command
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir1_after = ft_lstlast(cmd1->redirs)->content;
    mu_assert("heredoc content mismatch cmd1", verify_heredoc_content(redir1_after->target, expected_content) == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();

    return 0;
}

// Test 13: test that last status code is expanded correctly to 127 on second command
static int test_last_status_expanded_to_127_second_command(void)
{
    printf("Test: test_last_status_expanded_to_127\n");

    const char *user_input[] = {
        "line1", "$?", "EOF",
        NULL
    };
    const char *expected_content = "line1\n127\n";

    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 127);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_second_command("EOF", 0);

    set_here_docs(sh, pipe_head);

    // verify path vs expected for every command
    t_cmd *cmd2 = (t_cmd*)pipe_head->next->content;
    t_redir *redir2_after = ft_lstlast(cmd2->redirs)->content;
    mu_assert("heredoc content mismatch cmd2", verify_heredoc_content(redir2_after->target, expected_content) == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();

    return 0;
}

// Test 15: test that last status code is NOT expanded to 127 when quoted
static int test_last_status_not_expanded_when_quoted(void)
{
    printf("Test: test_last_status_not_expanded_when_quoted\n");

    const char *user_input[] = {
        "line1", "\"$?\"", "EOF",
        NULL
    };
    const char *expected_content = "line1\n\"$?\"\n";

    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 127);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 1);

    set_here_docs(sh, pipe_head);

    // verify path vs expected for every command
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir1_after = ft_lstlast(cmd1->redirs)->content;
    mu_assert("heredoc content mismatch cmd1", verify_heredoc_content(redir1_after->target, expected_content) == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();

    return 0;
}

// Test 16: miscellanea 6 commands, various here docs, expanded and not expanded, with and without last_status
static int test_miscellanea_6_commands(void)
{
    printf("Test: test_miscellanea_6_commands\n");

    const char *user_input[] = {
        "line1", "$?", "hey $USER", "EOF",
        "$?", "$?", "EOF",
        "$HOME and", "Invalid: $NON last status is: $?", "I'm $USER , last status was $? and I'm using $SHELL", "EOF",
        NULL
    };
    const char *expected_content1 = "line1\n127\nhey saalarco\n";
    const char *expected_content2 = "$?\n$?\n";
    const char *expected_content3 = "/home/saalarco and\nInvalid:  last status is: 127\nI'm saalarco , last status was 127 and I'm using /bin/bash\n";

    setup_readline_mock(user_input);

    t_shell *sh = create_test_shell(test_env, 127);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_various_heredocs("EOF", 0, "EOF", 1, "EOF", 0);

    set_here_docs(sh, pipe_head);

    // verify path vs expected for every command
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir1_after = ft_lstlast(cmd1->redirs)->content;
    mu_assert("heredoc content mismatch cmd1", verify_heredoc_content(redir1_after->target, expected_content1) == 0);

    t_cmd *cmd2 = (t_cmd*)pipe_head->next->content;
    t_redir *redir2_after = ft_lstlast(cmd2->redirs)->content;
    mu_assert("heredoc content mismatch cmd2", verify_heredoc_content(redir2_after->target, expected_content2) == 0);

    t_cmd *cmd4 = (t_cmd*)pipe_head->next->next->next->content;
    t_redir *redir4_after = ft_lstlast(cmd4->redirs)->content;
    mu_assert("heredoc content mismatch cmd4", verify_heredoc_content(redir4_after->target, expected_content3) == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();

    return 0;
}


// Test 17: Heredoc on first command, multiple lines, with expansion $ is keep as literal
static int test_heredoc_first_command_dollar_sign(void)
{
    printf("Test: test_heredoc_first_command_dollar_sign\n");

    const char *user_input[] = {"line1", "$", "EOF", NULL};
    const char *expected_content = "line1\n$\n";

    setup_readline_mock(user_input);
    
    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);
    
    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);
    
    set_here_docs(sh, pipe_head);
    
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir_after = ft_lstlast(cmd1->redirs)->content;
    
    mu_assert("fd changed when setting heredoc", redir_after->fd == 0);
    mu_assert("heredoc content mismatch", 
              verify_heredoc_content(redir_after->target, expected_content) == 0);
    
    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    
    return 0;
}

// Test 18: Heredoc on first command, multiple lines, with expansion $$ is keep as literal
static int test_heredoc_first_command_double_dollar_sign(void)
{
    printf("Test: test_heredoc_first_command_double_dollar_sign\n");

    const char *user_input[] = {"line1", "$$ HELLO $", "EOF", NULL};
    const char *expected_content = "line1\n$$ HELLO $\n";

    setup_readline_mock(user_input);
    
    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);
    
    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);
    
    set_here_docs(sh, pipe_head);
    
    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir_after = ft_lstlast(cmd1->redirs)->content;
    
    mu_assert("fd changed when setting heredoc", redir_after->fd == 0);
    mu_assert("heredoc content mismatch", 
              verify_heredoc_content(redir_after->target, expected_content) == 0);
    
    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    
    return 0;
}

// Test 19: Here doc on first command, but opens fails with eaccess, set_here_docs returns -1
static int test_heredoc_first_command_open_fail(void)
{
    printf("Test: test_heredoc_first_command_open_fail\n");

    const char *user_input[] = {"line1", "EOF", NULL};
    // const char *expected_content = "line1\n";

    setup_readline_mock(user_input);
    setup_open_fails_at_call(open_wrap_eaccess, 0);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);

    int result = set_here_docs(sh, pipe_head);
    mu_assert("set_here_docs failed", result == -1);

    t_cmd *cmd1 = (t_cmd*)pipe_head->content;
    t_redir *redir_after = ft_lstlast(cmd1->redirs)->content;

    mu_assert("fd changed when setting heredoc", redir_after->fd == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    syswrap_set_open(NULL);

    return 0;
}


// Test 20: Multiple commands with heredocs pipeline but opens fails at call 1
static int test_heredocs_multiple_commands_open_fails_at_1(void)
{
    printf("Test: test_heredocs_multiple_commands_open_fails_at_1\n");

    const char *user_input[] = {
        "$HOME", "EOF",
        "$$ hello this here_doc won't even be fetched because open is failing", "EOF",
        NULL
    };

    setup_readline_mock(user_input);
    setup_open_fails_at_call(open_wrap_eaccess, 1);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_various_heredocs("EOF", 0, "EOF", 0, "EOF", 0);

    int result = set_here_docs(sh, pipe_head);
    mu_assert("set_here_docs failed", result == -1);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    syswrap_set_open(NULL);

    return 0;
}


// Test 21: Multiple commands with heredocs pipeline but opens fails at call 2
static int test_heredocs_multiple_commands_open_fails_at_2(void)
{
    printf("Test: test_heredocs_multiple_commands_open_fails_at_2\n");

    const char *user_input[] = {
        "$HOME", "EOF",
        "abc", "EOF",
        "$$ hello this here_doc won't even be fetched because open is failing", "EOF",
        NULL
    };

    setup_readline_mock(user_input);
    setup_open_fails_at_call(open_wrap_eaccess, 2);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_various_heredocs("EOF", 0, "EOF", 0, "EOF", 0);

    int result = set_here_docs(sh, pipe_head);
    mu_assert("set_here_docs failed", result == -1);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    syswrap_set_open(NULL);

    return 0;
}

// Test 22: One command in pipeline with here doc with 1 line and expansion but readline fails at that first call
static int test_heredoc_one_command_pipeline_readline_fails(void)
{
    printf("Test: test_heredoc_one_command_pipeline_readline_fails\n");

    const char *user_input[] = {
        "$HOME", "EOF",
        NULL
    };

    setup_readline_mock(user_input);
    setup_readline_to_fail_at(user_input, 1);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_various_heredocs("EOF", 0, "EOF", 0, "EOF", 0);

    int result = set_here_docs(sh, pipe_head);
    mu_assert("set_here_docs failed", result == -1);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    syswrap_set_open(NULL);

    return 0;
}

// Test 23: One command in pipeline with here doc with various line and expansion but readline fails at second call
static int test_heredoc_one_command_pipeline_readline_fails_at_2(void)
{
    printf("Test: test_heredoc_one_command_pipeline_readline_fails_at_2\n");

    const char *user_input[] = {
        "This goes ok $HOME", "line2 will fail", "EOF",
        NULL
    };

    setup_readline_mock(user_input);
    setup_readline_to_fail_at(user_input, 1);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);

    int result = set_here_docs(sh, pipe_head);
    mu_assert("set_here_docs failed", result == -1);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    syswrap_set_open(NULL);

    return 0;
}


// Test 24: One command in pipeline with here doc with various line and expansion but readline fails at EOF
static int test_heredoc_one_command_pipeline_readline_fails_at_eof(void)
{
    printf("Test: test_heredoc_one_command_pipeline_readline_fails_at_eof\n");

    const char *user_input[] = {
        "This goes ok $HOME", "line2 will fail", "EOF",
        NULL
    };

    setup_readline_mock(user_input);
    setup_readline_to_fail_at(user_input, 2);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);

    int result = set_here_docs(sh, pipe_head);
    mu_assert("set_here_docs failed", result == -1);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    syswrap_set_open(NULL);

    return 0;
}

// Test 25: multiple here docs (so various commands with here doc) and readline fails in the middle of second command
static int test_heredoc_multiple_commands_pipeline_readline_fails(void)
{
    printf("Test: test_heredoc_multiple_commands_pipeline_readline_fails\n");

    const char *user_input[] = {
        "HD1 expands $HOME line 1", "HD1 line 2", "EOF",
        "HD2 expands $HOME line 1", "HD2 line 2 will fail readline", "EOF",
        NULL
    };

    setup_readline_mock(user_input);
    setup_readline_to_fail_at(user_input, 4);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_various_heredocs("EOF", 0, "EOF", 0, "EOF", 0);

    int result = set_here_docs(sh, pipe_head);
    mu_assert("set_here_docs failed", result == -1);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    syswrap_set_open(NULL);

    return 0;
}

//Test 26: On open fail, errno doesn't change on top level (from test itself)
static int test_heredoc_open_fail_errno_unchanged(void)
{
    printf("Test: test_heredoc_open_fail_errno_unchanged\n");

    const char *user_input[] = {
        "This goes ok $HOME", "line2 will fail", "EOF",
        NULL
    };

    setup_readline_mock(user_input);
    setup_open_fails_at_call(open_wrap_eaccess, 0);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);

    int result = set_here_docs(sh, pipe_head);
    mu_assert("set_here_docs failed", result == -1);

    mu_assert("errno changed on open fail", sh->last_errno == EACCES);
    mu_assert("last_error_op changed on open fail", strcmp(sh->last_err_op, "open") == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();
    syswrap_set_open(NULL);

    return 0;
}

// Test 27: On readline fail, errno hasn't change on top level (this test)
static int test_heredoc_readline_fail_errno_unchanged(void)
{
    printf("Test: test_heredoc_readline_fail_errno_unchanged\n");

    const char *user_input[] = {
        "This goes ok $HOME", "line2 will fail", "EOF",
        NULL
    };

    setup_readline_to_fail_at(user_input, 1);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);

    int result = set_here_docs(sh, pipe_head);
    mu_assert("set_here_docs failed", result == -1);
    mu_assert("errno changed on readline fail", sh->last_errno == ENOMEM);
    mu_assert("last_error_op changed on readline fail", strcmp(sh->last_err_op, "readline") == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    teardown_readline_mock();

    return 0;
}

// Test 28: On reading for /proc/stat read fails so errno is set
static int test_heredoc_read_proc_stat_fail(void)
{
    printf("Test: test_heredoc_read_proc_stat_fail\n");

    setup_read_fails(generic_read_mock);

    t_shell *sh = create_test_shell(test_env, 0);
    mu_assert("malloc shell failed", sh != NULL);

    t_list *pipe_head = build_generic_pipeline_with_heredoc_first_command("EOF", 0);

    int result = set_here_docs(sh, pipe_head);
    mu_assert("set_here_docs failed", result == -1);
    mu_assert("errno changed on /proc/stat read fail", sh->last_errno == EACCES);
    // mu_assert("last_error_op changed on /proc/stat read fail", strcmp(sh->last_err_op, "read") == 0);

    unlink_hds(pipe_head);
    free_cmds(pipe_head);
    free_shell(sh);
    syswrap_set_read(NULL);

    return 0;
}

int main(void)
{
    mu_run_test(test_various_cmds_no_here_doc_unchanges_returns_0);
    mu_run_test(test_heredoc_first_command_multiple_lines_no_expansion);
    mu_run_test(test_heredoc_first_command_multiple_lines_with_expansion);
    mu_run_test(test_various_cmds_with_heredoc_changes_returns_0);
    mu_run_test(test_empty_heredoc_user_introduces_delimiter);
    mu_run_test(test_empty_heredoc_user_introduces_delimiter_with_expansion);
    mu_run_test(test_various_cmds_with_empty_lines);
    mu_run_test(test_heredoc_second_command_multiple_lines_no_expansion);
    mu_run_test(test_heredoc_second_command_multiple_lines_with_expansion);
    mu_run_test(test_heredoc_multiple_commands_multiple_lines_no_expansion);
    mu_run_test(test_heredoc_multiple_commands_multiple_lines_with_expansion);
    mu_run_test(test_last_status_expanded_to_0);
    mu_run_test(test_last_status_expanded_to_127);
    mu_run_test(test_last_status_expanded_to_127_second_command);
    mu_run_test(test_last_status_not_expanded_when_quoted);
    mu_run_test(test_miscellanea_6_commands);
    mu_run_test(test_heredoc_first_command_dollar_sign);
    mu_run_test(test_heredoc_first_command_double_dollar_sign);

    // injected syscalls with errors (testing memory more than errors themselves)
    mu_run_test(test_heredoc_first_command_open_fail);
    mu_run_test(test_heredocs_multiple_commands_open_fails_at_1);
    mu_run_test(test_heredocs_multiple_commands_open_fails_at_2);
    mu_run_test(test_heredoc_one_command_pipeline_readline_fails);
    mu_run_test(test_heredoc_one_command_pipeline_readline_fails_at_2);
    mu_run_test(test_heredoc_one_command_pipeline_readline_fails_at_eof);
    mu_run_test(test_heredoc_multiple_commands_pipeline_readline_fails);
    mu_run_test(test_heredoc_read_proc_stat_fail);

    // custom tests to implement error handling file
    mu_run_test(test_heredoc_open_fail_errno_unchanged);
    mu_run_test(test_heredoc_readline_fail_errno_unchanged);

    // 

    mu_summary();
}