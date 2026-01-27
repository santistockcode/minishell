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

// ============================================================================
// Generic fork fails
// ============================================================================


static int fork_fail(void) {
    errno = EAGAIN;
    return -1;
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
// Generic access mock generator
// ============================================================================

typedef struct s_access_mock {
    int call_count;
    int fail_at;
} t_access_mock;

static t_access_mock g_access_mock = {0, 0};

static int access_mock(const char *path, int mode)
{
    (void)path;
    (void)mode;
    g_access_mock.call_count++;
    if (g_access_mock.call_count == g_access_mock.fail_at)
    {
        errno = EACCES; // Permission denied
        return (-1);
    }
    return access(path, mode);
}

static int access_mock_happy(const char *path, int mode)
{
    (void)path;
    (void)mode;
    g_access_mock.call_count++;
    if (g_access_mock.call_count == g_access_mock.fail_at)
    {
        return (0); // Simulate success
    }
    return access(path, mode);
}

static void setup_access_mock(int fail_at)
{
    g_access_mock.call_count = 0;
    g_access_mock.fail_at = fail_at;
    syswrap_set_access((t_access_fn)access_mock);
}

static void setup_access_mock_happy_path(int goes_ok_at)
{
    g_access_mock.call_count = 0;
    g_access_mock.fail_at = goes_ok_at;
    syswrap_set_access((t_access_fn)access_mock_happy);
}

static void teardown_access_mock(void)
{
    g_access_mock.call_count = 0;
    g_access_mock.fail_at = 0;
    syswrap_set_access(NULL);
}


// ============================================================================
// Generic dup2 mock generator
// ============================================================================

typedef struct s_dup2_mock {
    int call_count;
    int fail_at;
} t_dup2_mock;

static t_dup2_mock g_dup2_mock = {0, 0};

static int dup2_mock(int oldfd, int newfd)
{
    (void)oldfd;
    (void)newfd;
    g_dup2_mock.call_count++;
    if (g_dup2_mock.call_count == g_dup2_mock.fail_at)
    {
        errno = EBADF; // Bad file descriptor
        return -1;
    }
    return dup2(oldfd, newfd);
}

static void setup_dup2_mock(int fail_at)
{
    g_dup2_mock.call_count = 0;
    g_dup2_mock.fail_at = fail_at;
    syswrap_set_dup2((t_dup2_fn)dup2_mock);
}

static void teardown_dup2_mock(void)
{
    g_dup2_mock.call_count = 0;
    g_dup2_mock.fail_at = 0;
    syswrap_set_dup2(NULL);
}

// ============================================================================
// Generic execve mock generator
// ============================================================================

typedef struct s_execve_mock {
    int call_count;
    int fail_at;
    int failure_type;
} t_execve_mock;

static t_execve_mock g_execve_mock = {0, 0, 0};

static int execve_mock(const char *pathname, char *const argv[], char *const envp[])
{
    (void)pathname;
    (void)argv;
    (void)envp;
    g_execve_mock.call_count++;
    if (g_execve_mock.call_count == g_execve_mock.fail_at)
    {
        if (g_execve_mock.failure_type == 126)
            errno = EACCES;
        else if (g_execve_mock.failure_type == 127)
            errno = ENOENT;
        return -1;
    }
    return 0;
}

static void setup_execve_mock(int fail_at, int failure_type)
{
    g_execve_mock.call_count = 0;
    g_execve_mock.fail_at = fail_at;
    g_execve_mock.failure_type = failure_type;
    syswrap_set_execve((t_execve_fn)execve_mock);
}

static void teardown_execve_mock(void)
{
    g_execve_mock.call_count = 0;
    g_execve_mock.fail_at = 0;
    syswrap_set_execve(NULL);
}

// test prepare stage_io (I will do it later, no syscalls)

// test prepare redirs (later, is basically a wrap, no problemo I think)

// ============================================================================
// Tests for do_first_command
// ============================================================================

int	do_first_command(t_shell *sh, t_cmd *cmd, int *p);

// Happy path: first command with no redirections
static int test_do_first_command_happy_path_no_redir(void)
{
    printf("Test: test_do_first_command_happy_path_no_redir\n");
    t_shell *sh;
    int p[2];
    int status;
    pid_t first_pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "hello", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    
    pipe(p);
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return 0", result, 0);
    
    // Parent should have closed p[1]
    // Try to write to p[1] - should fail with EBADF
    char test_buf[] = "test";
    ssize_t write_result = write(p[1], test_buf, 4);
    mu_assert("p[1] should be closed in parent", write_result == -1 && errno == EBADF);
    
    
    // Read output from child
    char buf[128] = {0};
    read(p[0], buf, sizeof(buf) - 1);
    close(p[0]);
    
    // Wait for child to finish
    wait(&status);
    
    mu_assert("output should contain 'hello'", strstr(buf, "hello") != NULL);
    mu_assert("child should exit successfully", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Happy path: first command with input redirection
static int test_do_first_command_happy_path_with_redir_in(void)
{
    printf("Test: test_do_first_command_happy_path_with_redir_in\n");
    t_shell *sh;
    int p[2];
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(R_IN, "tests/unit/mock-files/infile.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    pipe(p);
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return 0", result, 0);
    
    // Read output from pipe
    char buf[128] = {0};
    read(p[0], buf, sizeof(buf) - 1);
    close(p[0]);
    
    mu_assert("output should contain content from infile", ft_strlen(buf) > 0);
    
    wait(&status);
    mu_assert("child should exit successfully", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Happy path: first command with output redirection (overrides pipe)
static int test_do_first_command_happy_path_with_redir_out(void)
{
    printf("Test: test_do_first_command_happy_path_with_redir_out\n");
    t_shell *sh;
    int p[2];
    int status;
    const char *out_file = "tests/unit/mock-files/test_first_out.txt";
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    unlink(out_file);
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "output_test", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    t_redir *redir = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    pipe(p);
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return 0", result, 0);

    
    wait(&status);
    
    // Check file was created with correct content
    int fd = open(out_file, O_RDONLY);
    mu_assert("output file should exist", fd >= 0);
    if (fd >= 0)
    {
        char file_buf[128] = {0};
        read(fd, file_buf, sizeof(file_buf) - 1);
        close(fd);
        mu_assert("file should contain 'output_test'", 
                  strstr(file_buf, "output_test") != NULL);
    }

    close(p[0]);

    unlink(out_file);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Happy path: first command with both input and output redirections
static int test_do_first_command_happy_path_with_both_redirs(void)
{
    printf("Test: test_do_first_command_happy_path_with_both_redirs\n");
    t_shell *sh;
    int p[2];
    int status;
    const char *out_file = "tests/unit/mock-files/test_first_both.txt";
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    unlink(out_file);
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir_in = make_redir(R_IN, "tests/unit/mock-files/infile.txt", 0, -1);
    t_redir *redir_out = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_in));
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_out));
    
    pipe(p);
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return 0", result, 0);
    
    close(p[0]);
    wait(&status);
    mu_assert("child should exit successfully", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    
    // Check output file exists and has content
    int fd = open(out_file, O_RDONLY);
    mu_assert("output file should exist", fd >= 0);
    if (fd >= 0)
    {
        char file_buf[128] = {0};
        ssize_t bytes = read(fd, file_buf, sizeof(file_buf) - 1);
        close(fd);
        mu_assert("file should have content from infile", bytes > 0);
    }
    
    unlink(out_file);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Fork failure
static int test_do_first_command_fork_failure(void)
{
    printf("Test: test_do_first_command_fork_failure\n");
    t_shell *sh;
    int p[2];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "test", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    pipe(p);
    
    // Mock fork to fail
    typedef pid_t (*t_fork_fn)(void);
    pid_t (*original_fork)(void) = fork;
    

    
    syswrap_set_fork((t_fork_fn)fork_fail);
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return -1 on fork failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    mu_assert_intcmp("last_errno should be EAGAIN", sh->last_errno, EAGAIN);
    
    syswrap_set_fork(NULL);
    
    close(p[0]);
    close(p[1]);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Open failure (input redirection)
static int test_do_first_command_open_failure_redir_in(void)
{
    printf("Test: test_do_first_command_open_failure_redir_in\n");
    t_shell *sh;
    int p[2];
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(R_IN, "tests/unit/mock-files/nonexistent.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    pipe(p);
    
    int result = do_first_command(sh, cmd, p);
    
    // Fork succeeds, but child exits with error
    mu_assert_intcmp("do_first_command should return 0 (fork succeeded)", result, 0);
    
    close(p[0]);
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 1", WEXITSTATUS(status), 1);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Open failure with mock (output redirection)
static int test_do_first_command_open_failure_redir_out_mock(void)
{
    printf("Test: test_do_first_command_open_failure_redir_out_mock\n");
    t_shell *sh;
    int p[2];
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "test", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    t_redir *redir = make_redir(R_OUT_TRUNC, "tests/unit/mock-files/out.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    pipe(p);
    
    // Mock open to fail on first call
    setup_open_fails_at_call(open_wrap_eaccess, 0);
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return 0 (fork succeeded)", result, 0);
    
    close(p[0]);
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 1", WEXITSTATUS(status), 1);
    
    syswrap_set_open(NULL);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Dup2 failure (first call - input)
static int test_do_first_command_dup2_failure_first_call(void)
{
    printf("Test: test_do_first_command_dup2_failure_first_call\n");
    t_shell *sh;
    int p[2];
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(R_IN, "tests/unit/mock-files/infile.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    pipe(p);
    
    setup_dup2_mock(1); // Fail on first dup2 call
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return 0 (fork succeeded)", result, 0);
    
    close(p[0]);
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", 
                     WEXITSTATUS(status), EXIT_FAILURE);
    
    teardown_dup2_mock();
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Dup2 failure (second call - output)
static int test_do_first_command_dup2_failure_second_call(void)
{
    printf("Test: test_do_first_command_dup2_failure_second_call\n");
    t_shell *sh;
    int p[2];
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir_in = make_redir(R_IN, "tests/unit/mock-files/infile.txt", 0, -1);
    t_redir *redir_out = make_redir(R_OUT_TRUNC, "tests/unit/mock-files/out.txt", 0, -1);
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_in));
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_out));
    
    pipe(p);
    
    setup_dup2_mock(2); // Fail on second dup2 call
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return 0 (fork succeeded)", result, 0);
    
    close(p[0]);
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", 
                     WEXITSTATUS(status), EXIT_FAILURE);
    
    teardown_dup2_mock();
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Dup failure in msh_save_fds
static int test_do_first_command_dup_failure_save_fds(void)
{
    printf("Test: test_do_first_command_dup_failure_save_fds\n");
    t_shell *sh;
    int p[2];
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "test", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    pipe(p);
    
    // Mock dup to fail (used in msh_save_fds)
    typedef int (*t_dup_fn)(int);
    
    setup_dup2_mock(1);
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return 0 (fork succeeded)", result, 0);
    
    close(p[0]);
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 1", WEXITSTATUS(status), 1);
    
    syswrap_set_dup(NULL);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Execve failure (126 - permission denied)
static int test_do_first_command_execve_failure_126(void)
{
    printf("Test: test_do_first_command_execve_failure_126\n");
    t_shell *sh;
    int p[2];
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    pipe(p);
    
    setup_execve_mock(1, 126);
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return 0 (fork succeeded)", result, 0);
    
    close(p[0]);
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 126", WEXITSTATUS(status), 126);
    
    teardown_execve_mock();
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Execve failure (127 - command not found)
static int test_do_first_command_execve_failure_127(void)
{
    printf("Test: test_do_first_command_execve_failure_127\n");
    t_shell *sh;
    int p[2];
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"nonexistent_command_xyz", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    
    pipe(p);
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return 0 (fork succeeded)", result, 0);
    
    close(p[0]);
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 127", WEXITSTATUS(status), 127);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Access failure in path resolution
static int test_do_first_command_access_failure_path_resolution(void)
{
    printf("Test: test_do_first_command_access_failure_path_resolution\n");
    t_shell *sh;
    int p[2];
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    pipe(p);
    
    setup_access_mock(1); // Fail on first access call
    
    int result = do_first_command(sh, cmd, p);
    
    mu_assert_intcmp("do_first_command should return 0 (fork succeeded)", result, 0);
    
    close(p[0]);
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    // Child will exit with 126 (permission denied from access)
    mu_assert("child exit status should be 126", WEXITSTATUS(status) == 126);
    
    teardown_access_mock();
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// ============================================================================
// Tests for do_middle_commands
// ============================================================================


// ============================================================================
// Tests for do_last_command
// ============================================================================


// test run pipeline (later)

int main(void)
{
    printf("=== Unit Tests for exec_pipeline ===\n\n");
    
    // do_first_command tests
    printf("\n--- do_first_command tests ---\n");
    mu_run_test(test_do_first_command_happy_path_no_redir);
    // Voy por aquí, PORQUE 0 stds at exit es un problema
    // mu_run_test(test_do_first_command_happy_path_with_redir_in);
    // mu_run_test(test_do_first_command_happy_path_with_redir_out);
    // mu_run_test(test_do_first_command_happy_path_with_both_redirs);
    // mu_run_test(test_do_first_command_fork_failure);
    // mu_run_test(test_do_first_command_open_failure_redir_in);
    // mu_run_test(test_do_first_command_open_failure_redir_out_mock);
    // mu_run_test(test_do_first_command_dup2_failure_first_call);
    // mu_run_test(test_do_first_command_dup2_failure_second_call);
    // mu_run_test(test_do_first_command_dup_failure_save_fds);
    // mu_run_test(test_do_first_command_execve_failure_126);
    // mu_run_test(test_do_first_command_execve_failure_127);
    // mu_run_test(test_do_first_command_access_failure_path_resolution);
    
    mu_summary();
    return 0;
}