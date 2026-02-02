 #include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

#include "../../../include/exec.h"
#include "../../../include/minishell.h"
#include "../../../include/syswrap.h"
#include "../../../Libft/include/libft.h"


t_list *create_middle_cmd_list_with_redir_in_on_second(void);
t_list *create_middle_cmd_list(void);
t_list *create_middle_cmd_list_with_redir_out_on_third(void);
void wait_for_all_children(int *statuses, int count);
int create_mock_pipe_with_data(const char *data);

int	run_pipeline(t_shell *sh, t_list *cmd_first, int nstages, pid_t *pid);

volatile static t_dup2_fn    s_dup2_fn    = NULL;

volatile sig_atomic_t g_exit_status = 0;

// Helper to create a standard 6-command pipeline list
static t_list *create_6_cmd_pipeline(void)
{
    t_list *cmds = NULL;
    
    const char *argv1[] = {"echo", "hello world", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    
    const char *argv3[] = {"cat", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    
    const char *argv4[] = {"cat", NULL};
    t_cmd *cmd4 = new_cmd_from_args(argv4, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd4));
    
    const char *argv5[] = {"cat", NULL};
    t_cmd *cmd5 = new_cmd_from_args(argv5, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd5));
    
    const char *argv6[] = {"wc", "-c", NULL};
    t_cmd *cmd6 = new_cmd_from_args(argv6, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd6));
    
    return cmds;
}

// ============================================================================
// Pipe failure tests
// ============================================================================

// Generic pipe mock
typedef struct s_pipe_mock {
    int call_count;
    int fail_at;
} t_pipe_mock;

static t_pipe_mock g_pipe_mock = {0, 0};

static int pipe_mock_nth(int pipefd[2])
{
    g_pipe_mock.call_count++;
    if (g_pipe_mock.call_count == g_pipe_mock.fail_at)
    {
        errno = EMFILE;  // Too many open files
        return -1;
    }
    return pipe(pipefd);
}

static void setup_pipe_mock(int fail_at)
{
    g_pipe_mock.call_count = 0;
    g_pipe_mock.fail_at = fail_at;
    syswrap_set_pipe((t_pipe_fn)pipe_mock_nth);
}

static void teardown_pipe_mock(void)
{
    g_pipe_mock.call_count = 0;
    g_pipe_mock.fail_at = 0;
    syswrap_set_pipe(NULL);
}

// ============================================================================
// Generic fork fails
// ============================================================================

static int fork_fail(void) {
    errno = EAGAIN;
    return -1;
}

// Setup fork mock to fail on 4th call
// (1st fork = first_cmd, 2nd fork = middle1, 3rd fork = middle2, 4th = fail)
typedef struct s_fork_mock {
    int call_count;
    int fail_at;
} t_fork_mock;

static t_fork_mock g_fork_mock = {0, 0};

static pid_t fork_mock_nth(void) {
    g_fork_mock.call_count++;
    if (g_fork_mock.call_count == g_fork_mock.fail_at) {
        errno = EAGAIN;
        return -1;
    }
    return fork();
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
// Generic dup mock generator
// ============================================================================

typedef struct s_dup_mock {
    int call_count;
    int fail_at;
} t_dup_mock;

static t_dup_mock g_dup_mock = {0, 0};

static int dup_mock(int oldfd)
{
    g_dup_mock.call_count++;
    if (g_dup_mock.call_count == g_dup_mock.fail_at)
    {
        errno = EBADF; // Bad file descriptor
        return -1;
    }
    return dup(oldfd);
}

static void setup_dup_mock(int fail_at)
{
    g_dup_mock.call_count = 0;
    g_dup_mock.fail_at = fail_at;
    syswrap_set_dup((t_dup_fn)dup_mock);
}

static void teardown_dup_mock(void)
{
    g_dup_mock.call_count = 0;
    g_dup_mock.fail_at = 0;
    syswrap_set_dup(NULL);
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
	// pid_t pidd = getpid();
	// fprintf(stderr, "pid IN PARENT: %d\n", pidd);
	// while(1)
	// 	sleep(10);
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

// PASS: no dup in child now since I don't store fds
// static int test_do_first_command_dup_failure_save_fds(void)
// {
//     printf("Test: test_do_first_command_dup_failure_save_fds\n");
//     t_shell *sh;
//     int p[2];
//     int status;
//     const char *test_env[] = {
//         "USER=saalarco",
//         "PATH=/usr/bin:/bin",
//         NULL
//     };
    
//     sh = create_test_shell(test_env, 0);
//     const char *argv[] = {"echo", "test", NULL};
//     t_cmd *cmd = new_cmd_from_args(argv, 2);
    
//     pipe(p);
    
//     // Mock dup to fail (used in msh_save_fds)
//     typedef int (*t_dup_fn)(int);
    
//     setup_dup_mock(1);
    
//     int result = do_first_command(sh, cmd, p);
    
//     mu_assert_intcmp("do_first_command should return 0 (fork succeeded)", result, 0);
    
//     close(p[0]);
//     wait(&status);
//     mu_assert("child should have exited normally", WIFEXITED(status));
//     if (is_builtin((char *) argv[0]))
//         mu_assert_intcmp("open failure should return -1", -1, status);
//     else
//         mu_assert_intcmp("open failure should return 1", 1, status);
//     syswrap_set_dup(NULL);
//     free_cmds(ft_lstnew(cmd));
//     free_shell(sh);
//     return (0);
// }

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

// Happy path: middle command with no redirections
static int test_do_middle_command_happy_path_no_redir(void)
{
    printf("Test: test_do_middle_command_happy_path_no_redir\n");
    t_shell *sh;
    int p[2];
    int in_fd;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    
    // Create input pipe and write some data
    int input_pipe[2];
    pipe(input_pipe);
    write(input_pipe[1], "test data\n", 10);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    // Create output pipe
    pipe(p);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return 0", result, 0);
    
    // Read output from pipe
    char buf[128] = {0};
    read(p[0], buf, sizeof(buf) - 1);
    close(p[0]);
    
    wait(&status);
    
    mu_assert("output should contain 'test data'", strstr(buf, "test data") != NULL);
    mu_assert("child should exit successfully", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Happy path: middle command with input redirection
static int test_do_middle_command_happy_path_with_redir_in(void)
{
    printf("Test: test_do_middle_command_happy_path_with_redir_in\n");
    t_shell *sh;
    int p[2];
    int in_fd;
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
    
    // Create dummy input pipe (will be overridden by redirection)
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    pipe(p);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return 0", result, 0);
    
    char buf[128] = {0};
    read(p[0], buf, sizeof(buf) - 1);
    close(p[0]);
    
    wait(&status);
    mu_assert("output should contain content from infile", ft_strlen(buf) > 0);
    mu_assert("child should exit successfully", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Happy path: middle command with output redirection
static int test_do_middle_command_happy_path_with_redir_out(void)
{
    printf("Test: test_do_middle_command_happy_path_with_redir_out\n");
    t_shell *sh;
    int p[2];
    int in_fd;
    int status;
    const char *out_file = "tests/unit/mock-files/test_middle_out.txt";
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    unlink(out_file);
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    int input_pipe[2];
    pipe(input_pipe);
    write(input_pipe[1], "middle output\n", 14);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    pipe(p);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return 0", result, 0);
    
    close(p[0]);
    wait(&status);
    
    int fd = open(out_file, O_RDONLY);
    mu_assert("output file should exist", fd >= 0);
    if (fd >= 0)
    {
        char file_buf[128] = {0};
        read(fd, file_buf, sizeof(file_buf) - 1);
        close(fd);
        mu_assert("file should contain 'middle output'", 
                  strstr(file_buf, "middle output") != NULL);
    }
    
    unlink(out_file);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Happy path: middle command with both redirections
static int test_do_middle_command_happy_path_with_both_redirs(void)
{
    printf("Test: test_do_middle_command_happy_path_with_both_redirs\n");
    t_shell *sh;
    int p[2];
    int in_fd;
    int status;
    const char *out_file = "tests/unit/mock-files/test_middle_both.txt";
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
    
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    pipe(p);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return 0", result, 0);
    
    close(p[0]);
    wait(&status);
    mu_assert("child should exit successfully", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    
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
static int test_do_middle_command_fork_failure(void)
{
    printf("Test: test_do_middle_command_fork_failure\n");
    t_shell *sh;
    int p[2];
    int in_fd;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    pipe(p);
    
    syswrap_set_fork((t_fork_fn)fork_fail);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return -1 on fork failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    mu_assert_intcmp("last_errno should be EAGAIN", sh->last_errno, EAGAIN);
    
    syswrap_set_fork(NULL);
    
    close(in_fd);
    close(p[0]);
    close(p[1]);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Open failure (input redirection)
static int test_do_middle_command_open_failure_redir_in(void)
{
    printf("Test: test_do_middle_command_open_failure_redir_in\n");
    t_shell *sh;
    int p[2];
    int in_fd;
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
    
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    pipe(p);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return 0 (fork succeeded)", result, 0);
    
    close(p[0]);
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 1", WEXITSTATUS(status), 1);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Open failure with mock (output redirection)
static int test_do_middle_command_open_failure_redir_out_mock(void)
{
    printf("Test: test_do_middle_command_open_failure_redir_out_mock\n");
    t_shell *sh;
    int p[2];
    int in_fd;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(R_OUT_TRUNC, "tests/unit/mock-files/out.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    int input_pipe[2];
    pipe(input_pipe);
    write(input_pipe[1], "data\n", 5);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    pipe(p);
    
    setup_open_fails_at_call(open_wrap_eaccess, 0);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return 0 (fork succeeded)", result, 0);
    
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
static int test_do_middle_command_dup2_failure_first_call(void)
{
    printf("Test: test_do_middle_command_dup2_failure_first_call\n");
    t_shell *sh;
    int p[2];
    int in_fd;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    
    int input_pipe[2];
    pipe(input_pipe);
    write(input_pipe[1], "data\n", 5);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    pipe(p);
    
    setup_dup2_mock(1);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return 0 (fork succeeded)", result, 0);
    
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
static int test_do_middle_command_dup2_failure_second_call(void)
{
    printf("Test: test_do_middle_command_dup2_failure_second_call\n");
    t_shell *sh;
    int p[2];
    int in_fd;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    
    int input_pipe[2];
    pipe(input_pipe);
    write(input_pipe[1], "data\n", 5);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    pipe(p);
    
    setup_dup2_mock(2);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return 0 (fork succeeded)", result, 0);
    
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

// PASS: no dup now in child cmds
// static int test_do_middle_command_dup_failure_save_fds(void)
// {
//     printf("Test: test_do_middle_command_dup_failure_save_fds\n");
//     t_shell *sh;
//     int p[2];
//     int in_fd;
//     int status;
//     const char *test_env[] = {
//         "USER=saalarco",
//         "PATH=/usr/bin:/bin",
//         NULL
//     };
    
//     sh = create_test_shell(test_env, 0);
//     const char *argv[] = {"cat", NULL};
//     t_cmd *cmd = new_cmd_from_args(argv, 1);
    
//     int input_pipe[2];
//     pipe(input_pipe);
//     write(input_pipe[1], "data\n", 5);
//     close(input_pipe[1]);
//     in_fd = input_pipe[0];
    
//     pipe(p);
    
//     setup_dup_mock(1);

//     t_list *cmds_list = ft_lstnew(cmd);
//     sh->cmds_start = cmds_list;
//     int result = do_middle_commands(sh, cmd, p, in_fd);
    
//     mu_assert_intcmp("do_middle_commands should return 0 (fork succeeded)", result, 0);
    
//     close(p[0]);
//     wait(&status);
//     mu_assert("child should have exited normally", WIFEXITED(status));
//     mu_assert_intcmp("child exit status should be 1", WEXITSTATUS(status), 1);
    
//     teardown_dup_mock();
//     free_cmds(cmds_list);
//     free_shell(sh);
//     return (0);
// }

// Execve failure (126 - permission denied)
static int test_do_middle_command_execve_failure_126(void)
{
    printf("Test: test_do_middle_command_execve_failure_126\n");
    t_shell *sh;
    int p[2];
    int in_fd;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    pipe(p);
    
    setup_execve_mock(1, 126);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return 0 (fork succeeded)", result, 0);
    
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
static int test_do_middle_command_execve_failure_127(void)
{
    printf("Test: test_do_middle_command_execve_failure_127\n");
    t_shell *sh;
    int p[2];
    int in_fd;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"nonexistent_command_xyz", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    pipe(p);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return 0 (fork succeeded)", result, 0);
    
    close(p[0]);
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 127", WEXITSTATUS(status), 127);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Access failure in path resolution
static int test_do_middle_command_access_failure_path_resolution(void)
{
    printf("Test: test_do_middle_command_access_failure_path_resolution\n");
    t_shell *sh;
    int p[2];
    int in_fd;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    pipe(p);
    
    setup_access_mock(1);
    
    int result = do_middle_commands(sh, cmd, p, in_fd);
    
    mu_assert_intcmp("do_middle_commands should return 0 (fork succeeded)", result, 0);
    
    close(p[0]);
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert("child exit status should be 126", WEXITSTATUS(status) == 126);
    
    teardown_access_mock();
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}


// ============================================================================
// Tests for do_middle_commands recursive (4 concatenated commands)
// ============================================================================


// Happy path: 3 middle commands (simulating 4-command pipeline)
// echo "hello world" | cat | tr 'a-z' 'A-Z' | wc -c
static int test_do_middle_command_recursive_happy_path_three_commands(void)
{
    printf("Test: test_do_middle_command_recursive_happy_path_three_commands\n");
    t_shell *sh;
    int in_fd;
    int new_pipe[2];
    int statuses[3];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    t_list *cmds = create_middle_cmd_list();
    
    // Simulate output from first command (echo "hello world")
    in_fd = create_mock_pipe_with_data("hello world\n");
    mu_assert("mock pipe should be created", in_fd >= 0);

    // Call do_middle_commands with list of 3 commands
    // It should recursively process all middle commands
    t_list *current = cmds;
    int result = 0;
    int prev_in_fd = in_fd;
    sh->cmds_start = cmds;
    while (current && current->next) // Process all but last as middle
    {
        t_cmd *cmd = (t_cmd *)current->content;
        pipe(new_pipe);
        
        result = do_middle_commands(sh, cmd, new_pipe, prev_in_fd);
        if (result != 0)
            break;
        
        prev_in_fd = new_pipe[0];
        current = current->next;
    }

    sh->cmds_start = NULL;
    mu_assert_intcmp("do_middle_commands should return 0 for all", result, 0);
    
    // Read final output
    char buf[128] = {0};
    read(prev_in_fd, buf, sizeof(buf) - 1);
    close(prev_in_fd);
    
    // Wait for all children
    wait_for_all_children(statuses, 2); // 2 middle commands processed
    
    // "hello world\n" = 12 chars, converted to uppercase, counted
    mu_assert("output should contain character count", ft_strlen(buf) > 0);
    
    for (int i = 0; i < 2; i++)
    {
        mu_assert("child should exit successfully", 
                  WIFEXITED(statuses[i]) && WEXITSTATUS(statuses[i]) == 0);
    }
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Fork failure on 2th fork call (during 2nd middle command in 3-middle pipeline)
static int test_do_middle_command_recursive_fork_failure_on_second_call(void)
{
    printf("Test: test_do_middle_command_recursive_fork_failure_on_second_call\n");
    t_shell *sh;
    int in_fd;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    int new_pipe[2];

    sh = create_test_shell(test_env, 0);
    t_list *cmds = create_middle_cmd_list();
    
    in_fd = create_mock_pipe_with_data("hello world\n");
    mu_assert("mock pipe should be created", in_fd >= 0);
    
    
    g_fork_mock.call_count = 0;
    g_fork_mock.fail_at = 2; // Fail when trying 2nd middle command
    syswrap_set_fork((t_fork_fn)fork_mock_nth);
    
    t_list *current = cmds;
    int result = 0;
    int prev_in_fd = in_fd;
    int cmd_count = 0;
    // special closure for middle commands
    sh->cmds_start = cmds;

    while (current && current->next)
    {
        t_cmd *cmd = (t_cmd *)current->content;
        pipe(new_pipe);
        
        result = do_middle_commands(sh, cmd, new_pipe, prev_in_fd);
        cmd_count++;
        
        if (result != 0)
        {
            close(new_pipe[0]);
            close(new_pipe[1]);
            break;
        }
        
        prev_in_fd = new_pipe[0];
        current = current->next;
    }
    sh->cmds_start = NULL;
    mu_assert_intcmp("do_middle_commands should return -1 on fork failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    mu_assert_intcmp("last_errno should be EAGAIN", sh->last_errno, EAGAIN);
    
    syswrap_set_fork(NULL);
    
    // Wait for any children that were created
    while (wait(NULL) > 0);
    
    close(prev_in_fd);
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Fork failure on 2th fork call (during 2nd middle command in 3-middle pipeline)
static int test_do_middle_command_recursive_fork_failure_on_third_call(void)
{
    printf("Test: test_do_middle_command_recursive_fork_failure_on_second_call\n");
    t_shell *sh;
    int in_fd;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    int new_pipe[2];

    sh = create_test_shell(test_env, 0);
    t_list *cmds = create_middle_cmd_list();
    
    in_fd = create_mock_pipe_with_data("hello world\n");
    mu_assert("mock pipe should be created", in_fd >= 0);
    
    
    g_fork_mock.call_count = 0;
    g_fork_mock.fail_at = 3; // Fail when trying 3nd middle command
    syswrap_set_fork((t_fork_fn)fork_mock_nth);
    
    t_list *current = cmds;
    int result = 0;
    int prev_in_fd = in_fd;
    int cmd_count = 0;
    // special closure for middle commands
    sh->cmds_start = cmds;

    while (current)
    {
        t_cmd *cmd = (t_cmd *)current->content;
        pipe(new_pipe);
        
        result = do_middle_commands(sh, cmd, new_pipe, prev_in_fd);
        cmd_count++;
        
        if (result != 0)
        {
            close(new_pipe[0]);
            close(new_pipe[1]);
            break;
        }
        
        prev_in_fd = new_pipe[0];
        current = current->next;
    }
    sh->cmds_start = NULL;
    mu_assert_intcmp("do_middle_commands should return -1 on fork failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    mu_assert_intcmp("last_errno should be EAGAIN", sh->last_errno, EAGAIN);
    
    syswrap_set_fork(NULL);
    
    // Wait for any children that were created
    while (wait(NULL) > 0);
    
    close(prev_in_fd);
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Open failure on input redirection during 2nd middle command
static int test_do_middle_command_recursive_open_failure_redir_in_on_second_call(void)
{
    printf("Test: test_do_middle_command_recursive_open_failure_redir_in_on_second_call\n");
    t_shell *sh;
    int in_fd;
    int statuses[3];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    // Create commands where 2nd has input redir to nonexistent file
    t_list *cmds = NULL;
    
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));

    const char *argv3[] = {"head", "-n", "1", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 3);
    t_redir *redir = make_redir(R_IN, "inexistant.md", 0, -1);
    cmd3->redirs = ft_lstnew(redir);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));


    
    // won't be executed
    // const char *argv4[] = {"wc", "-c", NULL};
    // t_cmd *cmd4 = new_cmd_from_args(argv4, 2);
    // ft_lstadd_back(&cmds, ft_lstnew(cmd4));
    
    in_fd = create_mock_pipe_with_data("hello world\n");
    
    t_list *current = cmds;
    int result = 0;
    int prev_in_fd = in_fd;
    int child_count = 0;

    sh->cmds_start = cmds;
    while (current)
    {
        t_cmd *cmd = (t_cmd *)current->content;
        int new_pipe[2];
        pipe(new_pipe);
        result = do_middle_commands(sh, cmd, new_pipe, prev_in_fd);
        child_count++;
        prev_in_fd = new_pipe[0];
        current = current->next;
    }
    sh->cmds_start = NULL;
    
    mu_assert_intcmp("do_middle_commands should return 0 (fork succeeded)", result, 0);
    
    close(prev_in_fd);
    fprintf(stderr, "Number of children created: %d\n", child_count);
    // Wait and check that 2nd child exited with error
    for (int i = 0; i < child_count; i++)
        wait(&statuses[i]);
    
    // ACHTUNG: this test doesn't work because head finishes before cat
    // mu_assert("first child should exit successfully", 
    //           WIFEXITED(statuses[0]) && WEXITSTATUS(statuses[0]) == 0);
    // mu_assert("second child should exit with error", 
    //           WIFEXITED(statuses[1]) && WEXITSTATUS(statuses[1]) == 1);

    // assert one of the two statuses is 1
    mu_assert("one of the children should exit with error",
              (WIFEXITED(statuses[0]) && WEXITSTATUS(statuses[0]) == 1) ||
              (WIFEXITED(statuses[1]) && WEXITSTATUS(statuses[1]) == 1));

    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Open failure on output redirection during 3rd middle command (mocked)
static int test_do_middle_command_recursive_open_failure_redir_out_on_third_call(void)
{
    printf("Test: test_do_middle_command_recursive_open_failure_redir_out_on_third_call\n");
    t_shell *sh;
    int in_fd;
    int statuses[3];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    t_list *cmds = create_middle_cmd_list_with_redir_out_on_third();
    
    in_fd = create_mock_pipe_with_data("hello world\n");
    
    // Setup open mock to fail on 2nd call (the output redir on 3rd command)
    // First open calls are for other things, adjust as needed
    setup_open_fails_at_call(open_wrap_eaccess, 0);
    
    t_list *current = cmds;
    int result = 0;
    int prev_in_fd = in_fd;
    int child_count = 0;
    
    // special closure
    sh->cmds_start = cmds;
    while (current && current->next)
    {
        t_cmd *cmd = (t_cmd *)current->content;
        int new_pipe[2];
        pipe(new_pipe);
        
        result = do_middle_commands(sh, cmd, new_pipe, prev_in_fd);
        child_count++;
        
        prev_in_fd = new_pipe[0];
        current = current->next;
    }
    
    mu_assert_intcmp("do_middle_commands should return 0 (fork succeeded)", result, 0);
    sh->cmds_start = NULL;
    close(prev_in_fd);
    
    for (int i = 0; i < child_count; i++)
        wait(&statuses[i]);
    
    // assert one of the two statuses is 1
    mu_assert("one of the children should exit with error",
              (WIFEXITED(statuses[0]) && WEXITSTATUS(statuses[0]) == 1) ||
              (WIFEXITED(statuses[1]) && WEXITSTATUS(statuses[1]) == 1));
        
    syswrap_set_open(NULL);
    unlink("tests/unit/mock-files/test_recursive_out.txt");
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Dup2 failure on 6th call (during stdin setup of 3rd middle command)
static int test_do_middle_command_recursive_dup2_failure_sixth_call(void)
{
    printf("Test: test_do_middle_command_recursive_dup2_failure_sixth_call\n");
    t_shell *sh;
    int in_fd;
    int statuses[3];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    t_list *cmds = create_middle_cmd_list();
    
    in_fd = create_mock_pipe_with_data("hello world\n");
    
    // Each middle command does ~2 dup2 calls (stdin, stdout)
    // dup2_mock not global so this fails on cat but also on head
    setup_dup2_mock(2);
    
    t_list *current = cmds;
    int result = 0;
    int prev_in_fd = in_fd;
    int child_count = 0;
    
    // special middle closure
    sh->cmds_start = cmds;
    while (current && current->next)
    {
        t_cmd *cmd = (t_cmd *)current->content;
        int new_pipe[2];
        pipe(new_pipe);
        
        result = do_middle_commands(sh, cmd, new_pipe, prev_in_fd);
        child_count++;
        
        prev_in_fd = new_pipe[0];
        current = current->next;
    }
    
    close(prev_in_fd);
    sh->cmds_start = NULL;
    
    for (int i = 0; i < child_count; i++)
        wait(&statuses[i]);
    
    // Check that one of the children exited with EXIT_FAILURE due to dup2
    int found_failure = 0;
    for (int i = 0; i < child_count; i++)
    {
        if (WIFEXITED(statuses[i]) && WEXITSTATUS(statuses[i]) == EXIT_FAILURE)
            found_failure = 1;
    }
    mu_assert("at least one child should exit with EXIT_FAILURE", found_failure);
    
    teardown_dup2_mock();
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Dup2 failure on 4th call (during stdout setup of 2nd middle command)
static int test_do_middle_command_recursive_dup2_failure_fourth_call(void)
{
    printf("Test: test_do_middle_command_recursive_dup2_failure_fourth_call\n");
    t_shell *sh;
    int in_fd;
    int statuses[3];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    t_list *cmds = create_middle_cmd_list();
    
    in_fd = create_mock_pipe_with_data("hello world\n");
    
    // on every child process first dup2 will fail
    setup_dup2_mock(1);
    
    t_list *current = cmds;
    int result = 0;
    int prev_in_fd = in_fd;
    int child_count = 0;

    // special closure
    sh->cmds_start = cmds;
    
    while (current)
    {
        t_cmd *cmd = (t_cmd *)current->content;
        int new_pipe[2];
        pipe(new_pipe);
        
        result = do_middle_commands(sh, cmd, new_pipe, prev_in_fd);
        child_count++;
        
        prev_in_fd = new_pipe[0];
        current = current->next;
    }
    
    close(prev_in_fd);
    sh->cmds_start = NULL;
    for (int i = 0; i < child_count; i++)
        wait(&statuses[i]);
    
    // 2nd child should fail due to dup2 error
    mu_assert("second child should exit with EXIT_FAILURE",
              WIFEXITED(statuses[1]) && WEXITSTATUS(statuses[1]) == EXIT_FAILURE);
    
    teardown_dup2_mock();
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Execve failure (126 - permission denied) on 3rd middle command
static int test_do_middle_command_recursive_execve_failure_on_third_call_126(void)
{
    printf("Test: test_do_middle_command_recursive_execve_failure_on_third_call_126\n");
    t_shell *sh;
    int in_fd;
    int statuses[3];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    t_list *cmds = create_middle_cmd_list();
    
    in_fd = create_mock_pipe_with_data("hello world\n");
    
    // Fail execve on 2nd call (2nd middle command)
    setup_execve_mock(1, 126);
    
    t_list *current = cmds;
    int result = 0;
    int prev_in_fd = in_fd;
    int child_count = 0;
    
    sh->cmds_start = cmds;
    while (current && current->next)
    {
        t_cmd *cmd = (t_cmd *)current->content;
        int new_pipe[2];
        pipe(new_pipe);
        
        result = do_middle_commands(sh, cmd, new_pipe, prev_in_fd);
        child_count++;
        
        prev_in_fd = new_pipe[0];
        current = current->next;
    }
    
    close(prev_in_fd);
    sh->cmds_start = NULL;
    for (int i = 0; i < child_count; i++)
        wait(&statuses[i]);
    
    // 2nd child should exit with 126 (permission denied)
    mu_assert("child with execve failure should exit with 126",
              WIFEXITED(statuses[1]) && WEXITSTATUS(statuses[1]) == 126);
    
    teardown_execve_mock();
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Execve failure (127 - command not found) on 3rd middle command
static int test_do_middle_command_recursive_execve_failure_on_third_call_127(void)
{
    printf("Test: test_do_middle_command_recursive_execve_failure_on_third_call_127\n");
    t_shell *sh;
    int in_fd;
    int statuses[3];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    // Create commands where 2nd middle is nonexistent
    t_list *cmds = NULL;
    const char *argv3[] = {"nonexistent_cmd_recursive_test", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    
    
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    
    const char *argv4[] = {"wc", "-c", NULL};
    t_cmd *cmd4 = new_cmd_from_args(argv4, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd4));
    
    in_fd = create_mock_pipe_with_data("hello world\n");
    
    t_list *current = cmds;
    int result = 0;
    int prev_in_fd = in_fd;
    int child_count = 0;
    

    // special middle closrue ñapa
    sh->cmds_start = cmds;
    while (current && current->next)
    {
        t_cmd *cmd = (t_cmd *)current->content;
        int new_pipe[2];
        pipe(new_pipe);
        
        result = do_middle_commands(sh, cmd, new_pipe, prev_in_fd);
        child_count++;
        
        prev_in_fd = new_pipe[0];
        current = current->next;
    }
    
    close(prev_in_fd);
    
    for (int i = 0; i < child_count; i++)
        wait(&statuses[i]);
    sh->cmds_start = NULL;
        // Wait and check that 2nd child exited with error
    for (int i = 0; i < child_count; i++)
        wait(&statuses[i]);

    // assert one of the two statuses is 1
    mu_assert("one of the children should exit with error",
              (WIFEXITED(statuses[0]) && WEXITSTATUS(statuses[0]) == 127) ||
              (WIFEXITED(statuses[1]) && WEXITSTATUS(statuses[1]) == 127));
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// ============================================================================
// Tests for do_last_command
// ============================================================================

// Happy path: last command with no redirections
static int test_do_last_command_happy_path_no_redir(void)
{
    printf("Test: test_do_last_command_happy_path_no_redir\n");
    t_shell *sh;
    pid_t pid;
    int in_fd;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    status = 120;
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    
    // Create input pipe with data
    in_fd = create_mock_pipe_with_data("last command test\n");
    mu_assert("mock pipe should be created", in_fd >= 0);
    
    // Capture stdout for verification
    int stdout_pipe[2];
    pipe(stdout_pipe);
    int saved_stdout = dup(STDOUT_FILENO);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    close(stdout_pipe[1]);
    
    int result = do_last_command(sh, cmd, in_fd, &pid);
    
    // Restore stdout BEFORE waiting (so child output goes to pipe)
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);  // Close saved_stdout only ONCE, here
    
    mu_assert_intcmp("do_last_command should return 0", result, 0);

    waitpid(pid, &status, 0);

    // Read captured output
    char buf[128] = {0};
    read(stdout_pipe[0], buf, sizeof(buf) - 1);
    close(stdout_pipe[0]);
        
    mu_assert("output should contain 'last command test'", strstr(buf, "last command test") != NULL);
    mu_assert("child should exit successfully", WIFEXITED(status) && WEXITSTATUS(status) == 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Happy path: last command with input redirection
static int test_do_last_command_happy_path_with_redir_in(void)
{
    printf("Test: test_do_last_command_happy_path_with_redir_in\n");
    t_shell *sh;
    pid_t pid;
    int in_fd;
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
    
    // Dummy input (will be overridden by redirection)
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    // Capture stdout
    int stdout_pipe[2];
    pipe(stdout_pipe);
    int saved_stdout = dup(STDOUT_FILENO);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    close(stdout_pipe[1]);

    int result = do_last_command(sh, cmd, in_fd, &pid);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    
    mu_assert_intcmp("do_last_command should return 0", result, 0);

    waitpid(pid, &status, 0);

    char buf[128] = {0};
    read(stdout_pipe[0], buf, sizeof(buf) - 1);
    close(stdout_pipe[0]);
    
    mu_assert("output should contain content from infile", ft_strlen(buf) > 0);
    mu_assert("child should exit successfully", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Happy path: last command with output redirection
static int test_do_last_command_happy_path_with_redir_out(void)
{
    printf("Test: test_do_last_command_happy_path_with_redir_out\n");
    t_shell *sh;
    pid_t pid;
    int in_fd;
    int status;
    const char *out_file = "tests/unit/mock-files/test_last_out.txt";
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    unlink(out_file);
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    in_fd = create_mock_pipe_with_data("output to file\n");

    int result = do_last_command(sh, cmd, in_fd, &pid);

    mu_assert_intcmp("do_last_command should return 0", result, 0);
    
    wait(&status);
    
    // Verify file content
    int fd = open(out_file, O_RDONLY);
    mu_assert("output file should exist", fd >= 0);
    if (fd >= 0)
    {
        char file_buf[128] = {0};
        read(fd, file_buf, sizeof(file_buf) - 1);
        close(fd);
        mu_assert("file should contain 'output to file'", 
                  strstr(file_buf, "output to file") != NULL);
    }
    
    mu_assert("child should exit successfully", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    
    unlink(out_file);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Happy path: last command with both redirections
static int test_do_last_command_happy_path_with_both_redirs(void)
{
    printf("Test: test_do_last_command_happy_path_with_both_redirs\n");
    t_shell *sh;
    pid_t pid;
    int in_fd;
    int status;
    const char *out_file = "tests/unit/mock-files/test_last_both.txt";
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
    
    // Dummy input (will be overridden)
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    int result = do_last_command(sh, cmd, in_fd, &pid);
    
    mu_assert_intcmp("do_last_command should return 0", result, 0);
    
    wait(&status);
    mu_assert("child should exit successfully", WIFEXITED(status) && WEXITSTATUS(status) == 0);
    
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
static int test_do_last_command_fork_failure(void)
{
    printf("Test: test_do_last_command_fork_failure\n");
    t_shell *sh;
    pid_t pid;
    int in_fd;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    syswrap_set_fork((t_fork_fn)fork_fail);

    int result = do_last_command(sh, cmd, in_fd, &pid);

    mu_assert_intcmp("do_last_command should return -1 on fork failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    mu_assert_intcmp("last_errno should be EAGAIN", sh->last_errno, EAGAIN);
    
    syswrap_set_fork(NULL);
    
    close(in_fd);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Open failure (input redirection)
static int test_do_last_command_open_failure_redir_in(void)
{
    printf("Test: test_do_last_command_open_failure_redir_in\n");
    t_shell *sh;
    pid_t pid;
    int in_fd;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(R_IN, "tests/unit/mock-files/nonexistent_last.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    int result = do_last_command(sh, cmd, in_fd, &pid);
    
    mu_assert_intcmp("do_last_command should return 0 (fork succeeded)", result, 0);
    
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 1", WEXITSTATUS(status), 1);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Open failure with mock (output redirection)
static int test_do_last_command_open_failure_redir_out_mock(void)
{
    printf("Test: test_do_last_command_open_failure_redir_out_mock\n");
    t_shell *sh;
    pid_t pid;
    int in_fd;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(R_OUT_TRUNC, "tests/unit/mock-files/out_last.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    in_fd = create_mock_pipe_with_data("data\n");
    
    setup_open_fails_at_call(open_wrap_eaccess, 0);

    int result = do_last_command(sh, cmd, in_fd, &pid);

    mu_assert_intcmp("do_last_command should return 0 (fork succeeded)", result, 0);
    
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 1", WEXITSTATUS(status), 1);
    
    syswrap_set_open(NULL);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// pass: no Dup on child processes for now
// static int test_do_last_command_dup_failure_save_fds(void)
// {
//     printf("Test: test_do_last_command_dup_failure_save_fds\n");
//     t_shell *sh;
//     int in_fd;
//     pid_t pid;
//     int status;
//     const char *test_env[] = {
//         "USER=saalarco",
//         "PATH=/usr/bin:/bin",
//         NULL
//     };
    
//     sh = create_test_shell(test_env, 0);
//     const char *argv[] = {"cat", NULL};
//     t_cmd *cmd = new_cmd_from_args(argv, 1);
    
//     in_fd = create_mock_pipe_with_data("data\n");
    
//     setup_dup_mock(1);
    
//     t_list *cmds_list = ft_lstnew(cmd);
//     sh->cmds_start = cmds_list;

//     int result = do_last_command(sh, cmd, in_fd, &pid);

//     mu_assert_intcmp("do_last_command should return 0 (fork succeeded)", result, 0);
    
//     wait(&status);
//     mu_assert("child should have exited normally", WIFEXITED(status));
//     mu_assert_intcmp("child exit status should be 1", WEXITSTATUS(status), 1);
    
//     teardown_dup_mock();
//     sh->cmds_start = NULL;
//     free_cmds(cmds_list);
//     free_shell(sh);
//     return (0);
// }

// Dup2 failure (first call - stdin)
static int test_do_last_command_dup2_failure_first_call(void)
{
    printf("Test: test_do_last_command_dup2_failure_first_call\n");
    t_shell *sh;
    int in_fd;
    pid_t pid;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    
    in_fd = create_mock_pipe_with_data("data\n");
    
    setup_dup2_mock(1);
    
    int result = do_last_command(sh, cmd, in_fd, &pid);
    
    mu_assert_intcmp("do_last_command should return 0 (fork succeeded)", result, 0);
    
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", 
                     WEXITSTATUS(status), EXIT_FAILURE);
    
    teardown_dup2_mock();
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Dup2 failure (second call - stdout)
static int test_do_last_command_dup2_failure_second_call(void)
{
    printf("Test: test_do_last_command_dup2_failure_second_call\n");
    t_shell *sh;
    int in_fd;
    pid_t pid;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir_out = make_redir(R_OUT_TRUNC, "tests/unit/mock-files/out.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir_out);
    
    in_fd = create_mock_pipe_with_data("data\n");
    
    setup_dup2_mock(2);

    int result = do_last_command(sh, cmd, in_fd, &pid);

    mu_assert_intcmp("do_last_command should return 0 (fork succeeded)", result, 0);
    
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", 
                     WEXITSTATUS(status), EXIT_FAILURE);
    
    teardown_dup2_mock();
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Execve failure (126 - permission denied)
static int test_do_last_command_execve_failure_126(void)
{
    printf("Test: test_do_last_command_execve_failure_126\n");
    t_shell *sh;
    int in_fd;
    pid_t pid;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    setup_execve_mock(1, 126);

    int result = do_last_command(sh, cmd, in_fd, &pid);

    mu_assert_intcmp("do_last_command should return 0 (fork succeeded)", result, 0);
    
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 126", WEXITSTATUS(status), 126);
    
    teardown_execve_mock();
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Execve failure (127 - command not found)
static int test_do_last_command_execve_failure_127(void)
{
    printf("Test: test_do_last_command_execve_failure_127\n");
    t_shell *sh;
    int in_fd;
    int status;
    pid_t pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"nonexistent_last_cmd", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    int result = do_last_command(sh, cmd, in_fd, &pid);
    
    mu_assert_intcmp("do_last_command should return 0 (fork succeeded)", result, 0);
    
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 127", WEXITSTATUS(status), 127);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// Access failure in path resolution
static int test_do_last_command_access_failure_path_resolution(void)
{
    printf("Test: test_do_last_command_access_failure_path_resolution\n");
    t_shell *sh;
    int in_fd;
    int status;
    pid_t pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    int input_pipe[2];
    pipe(input_pipe);
    close(input_pipe[1]);
    in_fd = input_pipe[0];
    
    setup_access_mock(1);

    int result = do_last_command(sh, cmd, in_fd, &pid);

    mu_assert_intcmp("do_last_command should return 0 (fork succeeded)", result, 0);
    
    wait(&status);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert("child exit status should be 126", WEXITSTATUS(status) == 126);
    
    teardown_access_mock();
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

// // ============================================================================
// // Tests for do_last_command as final stage in pipelines (two-stage)
// // ============================================================================

// Two-stage pipeline: echo | cat (happy path)
static int test_run_pipeline_last_of_two_happy_path(void)
{
    printf("Test: test_run_pipeline_last_of_two_happy_path\n");
    t_shell *sh;
    int p[2];
    int statuses[2];
    t_list *cmds = NULL;
    pid_t pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    // First command: echo "pipeline test"
    const char *argv1[] = {"echo", "pipeline test", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));

    // Last command: cat
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));



    pipe(p);

    // closure raro
    sh->cmds_start = cmds;
    // Execute first command
    int result1 = do_first_command(sh, cmd1, p);
    mu_assert_intcmp("do_first_command should return 0", result1, 0);
    
    // Capture stdout for last command
    int stdout_pipe[2];
    pipe(stdout_pipe);
    int saved_stdout = dup(STDOUT_FILENO);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    close(stdout_pipe[1]);
    
    // Execute last command
    int result2 = do_last_command(sh, cmd2, p[0], &pid);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    
    mu_assert_intcmp("do_last_command should return 0", result2, 0);

    // Read output
    char buf[128] = {0};
    read(stdout_pipe[0], buf, sizeof(buf) - 1);
    close(stdout_pipe[0]);
    
    mu_assert("output should contain 'pipeline test'", strstr(buf, "pipeline test") != NULL);
    // Wait for both children
    wait(&statuses[0]);
    wait(&statuses[1]);

    // Check that both children exited successfully
    mu_assert("first child should exit successfully", 
              WIFEXITED(statuses[0]) && WEXITSTATUS(statuses[0]) == 0);
    mu_assert("second child should exit successfully", 
              WIFEXITED(statuses[1]) && WEXITSTATUS(statuses[1]) == 0);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Two-stage pipeline with output redir on last: echo | cat > file
static int test_run_pipeline_last_of_two_with_redir_on_last(void)
{
    printf("Test: test_run_pipeline_last_of_two_with_redir_on_last\n");
    t_shell *sh;
    int p[2];
    t_list *cmds;
    pid_t   pid;
    int statuses[2];
    const char *out_file = "tests/unit/mock-files/pipeline_two_out.txt";
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    cmds = NULL;
    unlink(out_file);
    
    sh = create_test_shell(test_env, 0);
    
    const char *argv1[] = {"echo", "redir pipeline", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));

    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    t_redir *redir = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    cmd2->redirs = ft_lstnew(redir);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));

    sh->cmds_start = cmds;
    pipe(p);
    int result1 = do_first_command(sh, cmd1, p);
    mu_assert_intcmp("do_first_command should return 0", result1, 0);
    
    int result2 = do_last_command(sh, cmd2, p[0], &pid);
    mu_assert_intcmp("do_last_command should return 0", result2, 0);
    
    wait(&statuses[0]);
    wait(&statuses[1]);
    
    // Verify file content
    int fd = open(out_file, O_RDONLY);
    mu_assert("output file should exist", fd >= 0);
    if (fd >= 0)
    {
        char file_buf[128] = {0};
        read(fd, file_buf, sizeof(file_buf) - 1);
        close(fd);
        mu_assert("file should contain 'redir pipeline'", 
                  strstr(file_buf, "redir pipeline") != NULL);
    }
    
    unlink(out_file);
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Two-stage pipeline with fork failure on last
static int test_run_pipeline_last_of_two_fork_failure_on_last(void)
{
    printf("Test: test_run_pipeline_last_of_two_fork_failure_on_last\n");
    t_shell *sh;
    pid_t   pid;
    int p[2];
    int status;
    t_list *cmds;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = NULL;
    const char *argv1[] = {"echo", "test", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    sh->cmds_start = cmds;
    pipe(p);
    
    int result1 = do_first_command(sh, cmd1, p);
    mu_assert_intcmp("do_first_command should return 0", result1, 0);
    
    // Make fork fail for last command
    g_fork_mock.call_count = 0;
    g_fork_mock.fail_at = 1;
    syswrap_set_fork((t_fork_fn)fork_mock_nth);
    
    int result2 = do_last_command(sh, cmd2, p[0], &pid);
    
    mu_assert_intcmp("do_last_command should return -1", result2, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    
    syswrap_set_fork(NULL);
    
    // Wait for first child
    wait(&status);
    
    close(p[0]);
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Two-stage pipeline with execve 126 on last
static int test_run_pipeline_last_of_two_execve_failure_on_last_126(void)
{
    printf("Test: test_run_pipeline_last_of_two_execve_failure_on_last_126\n");
    t_shell *sh;
    int p[2];
    pid_t   pid;
    t_list *cmds = NULL;
    int statuses[2];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    const char *argv1[] = {"echo", "test", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    sh->cmds_start = cmds;
    pipe(p);
    
    int result1 = do_first_command(sh, cmd1, p);
    mu_assert_intcmp("do_first_command should return 0", result1, 0);
    
    setup_execve_mock(1, 126);
    
    int result2 = do_last_command(sh, cmd2, p[0], &pid);
    mu_assert_intcmp("do_last_command should return 0 (fork succeeded)", result2, 0);
    
    wait(&statuses[0]);
    wait(&statuses[1]);
    
    // One should be 126
    int found_126 = (WIFEXITED(statuses[0]) && WEXITSTATUS(statuses[0]) == 126) ||
                    (WIFEXITED(statuses[1]) && WEXITSTATUS(statuses[1]) == 126);
    mu_assert("one child should exit with 126", found_126);
    
    teardown_execve_mock();
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Two-stage pipeline with execve 127 on last
static int test_run_pipeline_last_of_two_execve_failure_on_last_127(void)
{
    printf("Test: test_run_pipeline_last_of_two_execve_failure_on_last_127\n");
    t_shell *sh;
    int p[2];
    t_list *cmds = NULL;
    pid_t   pid;
    int statuses[2];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    const char *argv1[] = {"echo", "test", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    const char *argv2[] = {"nonexistent_pipeline_cmd", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    sh->cmds_start = cmds;
    pipe(p);
    
    int result1 = do_first_command(sh, cmd1, p);
    int result2 = do_last_command(sh, cmd2, p[0], &pid);

    mu_assert_intcmp("do_first_command should return 0", result1, 0);
    mu_assert_intcmp("do_last_command should return 0", result2, 0);
    
    wait(&statuses[0]);
    wait(&statuses[1]);
    
    int found_127 = (WIFEXITED(statuses[0]) && WEXITSTATUS(statuses[0]) == 127) ||
                    (WIFEXITED(statuses[1]) && WEXITSTATUS(statuses[1]) == 127);
    mu_assert("one child should exit with 127", found_127);

    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Two-stage pipeline with dup2 failure on last
static int test_run_pipeline_last_of_two_dup2_failure_on_last(void)
{
    printf("Test: test_run_pipeline_last_of_two_dup2_failure_on_last\n");
    t_shell *sh;
    pid_t   pid;
    t_list *cmds = NULL;
    int p[2];
    int statuses[2];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    const char *argv1[] = {"echo", "test", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    sh->cmds_start = cmds;
    pipe(p);
    
    int result1 = do_first_command(sh, cmd1, p);
    mu_assert_intcmp("do_first_command should return 0", result1, 0);
    
    // dup2 mock resets per-process, so this will affect the last command's child
    setup_dup2_mock(1);

    int result2 = do_last_command(sh, cmd2, p[0], &pid);
    mu_assert_intcmp("do_last_command should return 0 (fork succeeded)", result2, 0);
    
    wait(&statuses[0]);
    wait(&statuses[1]);
    
    int found_failure = (WIFEXITED(statuses[0]) && WEXITSTATUS(statuses[0]) == EXIT_FAILURE) ||
                        (WIFEXITED(statuses[1]) && WEXITSTATUS(statuses[1]) == EXIT_FAILURE);
    mu_assert("one child should exit with EXIT_FAILURE", found_failure);
    
    teardown_dup2_mock();
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

//happy path

static int test_run_pipeline_last_of_three_happy_path(void)
{
    printf("Test: test_run_pipeline_last_of_three_happy_path\n");
    t_shell *sh;
    int p1[2], p2[2];
    int statuses[3];
    pid_t   pid;
    t_list *cmds = NULL;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    const char *argv1[] = {"echo", "test", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    const char *argv3[] = {"wc", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    sh->cmds_start = cmds;
    pipe(p1);
    do_first_command(sh, cmd1, p1);
    
    pipe(p2);
    do_middle_commands(sh, cmd2, p2, p1[0]);

    int result3 = do_last_command(sh, cmd3, p2[0], &pid);

    mu_assert_intcmp("do_last_command should return 0", result3, 0);
    
    wait(&statuses[0]);
    wait(&statuses[1]);
    wait(&statuses[2]);
    
    int found_success = (WIFEXITED(statuses[0]) && WEXITSTATUS(statuses[0]) == 0) &&
                        (WIFEXITED(statuses[1]) && WEXITSTATUS(statuses[1]) == 0) &&
                        (WIFEXITED(statuses[2]) && WEXITSTATUS(statuses[2]) == 0);
    mu_assert("all children should exit with 0", found_success);

    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Three-stage pipeline with output redir on last
static int test_run_pipeline_last_of_three_with_redir_on_last(void)
{
    printf("Test: test_run_pipeline_last_of_three_with_redir_on_last\n");
    t_shell *sh;
    int p1[2], p2[2];
    t_list *cmds = NULL;
    pid_t   pid;
    int statuses[3];
    const char *out_file = "tests/unit/mock-files/pipeline_three_out.txt";
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    unlink(out_file);
    
    sh = create_test_shell(test_env, 0);
    
    const char *argv1[] = {"echo", "three redir", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    const char *argv3[] = {"wc", "-c", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    t_redir *redir = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    cmd3->redirs = ft_lstnew(redir);
    sh->cmds_start = cmds;
    pipe(p1);
    int result1 = do_first_command(sh, cmd1, p1);
    
    pipe(p2);
    int result2 = do_middle_commands(sh, cmd2, p2, p1[0]);

    int result3 = do_last_command(sh, cmd3, p2[0], &pid);

    mu_assert_intcmp("all commands should return 0", result1 + result2 + result3, 0);
    
    for (int i = 0; i < 3; i++)
        wait(&statuses[i]);
    
    int fd = open(out_file, O_RDONLY);
    mu_assert("output file should exist", fd >= 0);
    if (fd >= 0)
    {
        char file_buf[128] = {0};
        read(fd, file_buf, sizeof(file_buf) - 1);
        close(fd);
        mu_assert("file should have character count", ft_strlen(file_buf) > 0);
    }
    
    unlink(out_file);
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Three-stage pipeline with fork failure on last
static int test_run_pipeline_last_of_three_fork_failure_on_last(void)
{
    printf("Test: test_run_pipeline_last_of_three_fork_failure_on_last\n");
    t_shell *sh;
    int p1[2], p2[2];
    int statuses[2];
    pid_t   pid;
    t_list *cmds = NULL;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    const char *argv1[] = {"echo", "test", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    const char *argv3[] = {"wc", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));

    sh->cmds_start = cmds;
    
    pipe(p1);
    do_first_command(sh, cmd1, p1);
    
    pipe(p2);
    do_middle_commands(sh, cmd2, p2, p1[0]);
    
    // Fail fork on last
    g_fork_mock.call_count = 0;
    g_fork_mock.fail_at = 1;
    syswrap_set_fork((t_fork_fn)fork_mock_nth);

    int result3 = do_last_command(sh, cmd3, p2[0], &pid);

    mu_assert_intcmp("do_last_command should return -1", result3, -1);
    
    syswrap_set_fork(NULL);
    
    wait(&statuses[0]);
    wait(&statuses[1]);
    
    close(p2[0]);
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Three-stage pipeline with execve 126 on last
static int test_run_pipeline_last_of_three_execve_failure_on_last_126(void)
{
    printf("Test: test_run_pipeline_last_of_three_execve_failure_on_last_126\n");
    t_shell *sh;
    int p1[2], p2[2];
    int statuses[3];
    t_list *cmds = NULL;
    pid_t   pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    const char *argv1[] = {"echo", "test", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    const char *argv3[] = {"wc", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));

    sh->cmds_start = cmds;

    pipe(p1);
    do_first_command(sh, cmd1, p1);
    
    pipe(p2);
    do_middle_commands(sh, cmd2, p2, p1[0]);
    
    setup_execve_mock(1, 126);
    do_last_command(sh, cmd3, p2[0], &pid);

    for (int i = 0; i < 3; i++)
        wait(&statuses[i]);
    
    int found_126 = 0;
    for (int i = 0; i < 3; i++)
    {
        if (WIFEXITED(statuses[i]) && WEXITSTATUS(statuses[i]) == 126)
            found_126 = 1;
    }
    mu_assert("one child should exit with 126", found_126);
    
    teardown_execve_mock();
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Three-stage pipeline with execve 127 on last
static int test_run_pipeline_last_of_three_execve_failure_on_last_127(void)
{
    printf("Test: test_run_pipeline_last_of_three_execve_failure_on_last_127\n");
    t_shell *sh;
    t_list *cmds = NULL;
    pid_t   pid;
    int p1[2], p2[2];
    int statuses[3];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    const char *argv1[] = {"echo", "test", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    const char *argv3[] = {"nonexistent_three_cmd", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));

    sh->cmds_start = cmds;

    pipe(p1);
    do_first_command(sh, cmd1, p1);
    
    pipe(p2);
    do_middle_commands(sh, cmd2, p2, p1[0]);

    do_last_command(sh, cmd3, p2[0], &pid);

    for (int i = 0; i < 3; i++)
        wait(&statuses[i]);
    
    int found_127 = 0;
    for (int i = 0; i < 3; i++)
    {
        if (WIFEXITED(statuses[i]) && WEXITSTATUS(statuses[i]) == 127)
            found_127 = 1;
    }
    mu_assert("one child should exit with 127", found_127);

    teardown_execve_mock();
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Three-stage pipeline with dup2 failure on last
static int test_run_pipeline_last_of_three_dup2_failure_on_last(void)
{
    printf("Test: test_run_pipeline_last_of_three_dup2_failure_on_last\n");
    t_shell *sh;
    int p1[2], p2[2];
    t_list  *cmds = NULL;
    pid_t   pid;
    int statuses[3];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    const char *argv1[] = {"echo", "test", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    const char *argv3[] = {"wc", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));

    sh->cmds_start = cmds;

    pipe(p1);
    do_first_command(sh, cmd1, p1);
    
    pipe(p2);
    do_middle_commands(sh, cmd2, p2, p1[0]);
    
    setup_dup2_mock(1);
    do_last_command(sh, cmd3, p2[0], &pid);
    
    for (int i = 0; i < 3; i++)
        wait(&statuses[i]);
    
    int found_failure = 0;
    for (int i = 0; i < 3; i++)
    {
        if (WIFEXITED(statuses[i]) && WEXITSTATUS(statuses[i]) == EXIT_FAILURE)
            found_failure = 1;
    }
    mu_assert("one child should exit with EXIT_FAILURE", found_failure);
    
    teardown_dup2_mock();
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// // ============================================================================
// // Additional integration-like cases
// // ============================================================================

// Test that intermediate failures affect the last command's input


static int test_run_pipeline_last_of_three_intermediate_failures_affect_last(void)
{
    printf("Test: test_run_pipeline_last_of_three_intermediate_failures_affect_last\n");
    t_shell *sh;
    pid_t   pid;
    t_list *cmds = NULL;
    int p1[2], p2[2];
    int statuses[3];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    // First: echo
    const char *argv1[] = {"echo", "test data", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    // Middle: nonexistent command (will fail with 127)
    const char *argv2[] = {"nonexistent_middle_cmd", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    // Last: wc (will receive no input because middle failed)
    const char *argv3[] = {"wc", "-c", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));

    sh->cmds_start = cmds;

    pipe(p1);
    do_first_command(sh, cmd1, p1);
    
    pipe(p2);
    do_middle_commands(sh, cmd2, p2, p1[0]);
    
    // Capture stdout
    int stdout_pipe[2];
    pipe(stdout_pipe);
    int saved_stdout = dup(STDOUT_FILENO);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    close(stdout_pipe[1]);

    do_last_command(sh, cmd3, p2[0], &pid);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    
    for (int i = 0; i < 3; i++)
        wait(&statuses[i]);
    
    // Middle should have failed with 127
    int found_127 = 0;
    for (int i = 0; i < 3; i++)
    {
        if (WIFEXITED(statuses[i]) && WEXITSTATUS(statuses[i]) == 127)
            found_127 = 1;
    }
    mu_assert("middle command should have failed with 127", found_127);
    
    // wc should output "0" or nothing since middle failed
    char buf[128] = {0};
    read(stdout_pipe[0], buf, sizeof(buf) - 1);
    close(stdout_pipe[0]);
    
    // Output should be 0 or empty (no data passed through failed middle)
    mu_assert("last command should receive no/minimal data", 
              ft_strlen(buf) == 0 || strstr(buf, "0") != NULL);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}


// ============================================================================
// Tests for run_pipeline with 6 commands
// ============================================================================

// 6-stage pipeline: echo "hello world" | cat | cat | cat | cat | wc -c
static int test_run_pipeline_6_cmds_pipe_happy_path(void)
{
    printf("Test: test_run_pipeline_6_cmds_pipe_happy_path\n");
    t_shell *sh;
    t_list *cmds = NULL;
    pid_t last_pid;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    // cmd1: echo "hello world"
    const char *argv1[] = {"echo", "hello world", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    
    // cmd2: cat
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    
    // cmd3: cat
    const char *argv3[] = {"cat", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    
    // cmd4: cat
    const char *argv4[] = {"cat", NULL};
    t_cmd *cmd4 = new_cmd_from_args(argv4, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd4));
    
    // cmd5: cat
    const char *argv5[] = {"cat", NULL};
    t_cmd *cmd5 = new_cmd_from_args(argv5, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd5));
    
    // cmd6: wc -c (count characters)
    const char *argv6[] = {"wc", "-c", NULL};
    t_cmd *cmd6 = new_cmd_from_args(argv6, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd6));
    
    // Capture stdout for verification
    int stdout_pipe[2];
    pipe(stdout_pipe);
    fcntl(stdout_pipe[0], F_SETFD, FD_CLOEXEC);

    int saved_stdout = dup(STDOUT_FILENO);
    fcntl(saved_stdout, F_SETFD, FD_CLOEXEC); 

    dup2(stdout_pipe[1], STDOUT_FILENO);
    close(stdout_pipe[1]);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    // Restore stdout
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    
    mu_assert_intcmp("run_pipeline should return 0", result, 0);
    
    // Wait for all 6 children
    int statuses[6];
    for (int i = 0; i < 6; i++)
        wait(&statuses[i]);
    
    // Read output from wc -c
    char buf[128] = {0};
    read(stdout_pipe[0], buf, sizeof(buf) - 1);
    close(stdout_pipe[0]);
    
    // "hello world\n" = 12 characters
    // wc -c outputs the count (possibly with whitespace padding)
    mu_assert("output should contain character count", ft_strlen(buf) > 0);
    mu_assert("output should contain '12'", strstr(buf, "12") != NULL);
    
    // All children should exit successfully
    int all_success = 1;
    for (int i = 0; i < 6; i++)
    {
        if (!WIFEXITED(statuses[i]) || WEXITSTATUS(statuses[i]) != 0)
            all_success = 0;
    }
    mu_assert("all 6 children should exit with status 0", all_success);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}


static int test_run_pipeline_6_cmds_pipe_happy_path_redir_out(void)
{
    printf("Test: test_run_pipeline_6_cmds_pipe_happy_path_redir_out\n");
    t_shell *sh;
    t_list *cmds = NULL;
    pid_t last_pid;
    const char *out_file = "tests/unit/mock-files/pipeline_6_out.txt";
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    // Clean up any previous test file
    unlink(out_file);
    
    sh = create_test_shell(test_env, 0);
    
    // cmd1: echo "hello world"
    const char *argv1[] = {"echo", "hello world", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    
    // cmd2: cat
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    
    // cmd3: cat
    const char *argv3[] = {"cat", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    
    // cmd4: cat
    const char *argv4[] = {"cat", NULL};
    t_cmd *cmd4 = new_cmd_from_args(argv4, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd4));
    
    // cmd5: cat
    const char *argv5[] = {"cat", NULL};
    t_cmd *cmd5 = new_cmd_from_args(argv5, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd5));
    
    // cmd6: wc -c > outfile (count characters, redirect to file)
    const char *argv6[] = {"wc", "-c", NULL};
    t_cmd *cmd6 = new_cmd_from_args(argv6, 2);
    t_redir *redir = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    cmd6->redirs = ft_lstnew(redir);
    ft_lstadd_back(&cmds, ft_lstnew(cmd6));
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return 0", result, 0);
    
    // Wait for all 6 children
    int statuses[6];
    for (int i = 0; i < 6; i++)
        wait(&statuses[i]);
    
    // Verify output file exists and contains correct count
    int fd = open(out_file, O_RDONLY);
    mu_assert("output file should exist", fd >= 0);
    
    char file_buf[128] = {0};
    if (fd >= 0)
    {
        read(fd, file_buf, sizeof(file_buf) - 1);
        close(fd);
    }
    
    // "hello world\n" = 12 characters
    mu_assert("file should contain character count", ft_strlen(file_buf) > 0);
    mu_assert("file should contain '12'", strstr(file_buf, "12") != NULL);
    
    // All children should exit successfully
    int all_success = 1;
    for (int i = 0; i < 6; i++)
    {
        if (!WIFEXITED(statuses[i]) || WEXITSTATUS(statuses[i]) != 0)
            all_success = 0;
    }
    mu_assert("all 6 children should exit with status 0", all_success);
    
    // Clean up test file
    unlink(out_file);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Pipe fails at first command (before any fork)
static int test_run_pipeline_6_cmds_pipe_fails_at_first_cmd(void)
{
    printf("Test: test_run_pipeline_6_cmds_pipe_fails_at_first_cmd\n");
    t_shell *sh;
    t_list *cmds;
    pid_t last_pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = create_6_cmd_pipeline();
    
    // Fail on first pipe() call
    setup_pipe_mock(1);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return -1 on pipe failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'pipe'", sh->last_err_op, PIPE_OP);
    mu_assert_intcmp("last_errno should be EMFILE", sh->last_errno, EMFILE);
    
    teardown_pipe_mock();
    
    // No children should have been created
    // (wait should return -1 with ECHILD)
    int wait_result = wait(NULL);
    mu_assert("no children should exist", wait_result == -1 && errno == ECHILD);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Pipe fails at middle command 1 (2nd pipe() call, between cmd1 and cmd2)
static int test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_1(void)
{
    printf("Test: test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_1\n");
    t_shell *sh;
    t_list *cmds;
    pid_t last_pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = create_6_cmd_pipeline();
    
    // Fail on 2nd pipe() call (for first middle command)
    setup_pipe_mock(2);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return -1 on pipe failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'pipe'", sh->last_err_op, PIPE_OP);
    
    teardown_pipe_mock();
    
    // Wait for any children that were created (first command)
    while (wait(NULL) > 0)
        ;
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Pipe fails at middle command 2 (3rd pipe() call)
static int test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_2(void)
{
    printf("Test: test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_2\n");
    t_shell *sh;
    t_list *cmds;
    pid_t last_pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = create_6_cmd_pipeline();
    
    // Fail on 3rd pipe() call (for second middle command)
    setup_pipe_mock(3);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return -1 on pipe failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'pipe'", sh->last_err_op, PIPE_OP);
    
    teardown_pipe_mock();
    
    // Wait for children (first + first middle)
    while (wait(NULL) > 0)
        ;
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Pipe fails at middle command 3 (4th pipe() call)
static int test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_3(void)
{
    printf("Test: test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_3\n");
    t_shell *sh;
    t_list *cmds;
    pid_t last_pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = create_6_cmd_pipeline();
    
    // Fail on 4th pipe() call (for third middle command)
    setup_pipe_mock(4);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return -1 on pipe failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'pipe'", sh->last_err_op, PIPE_OP);
    
    teardown_pipe_mock();
    
    // Wait for children (first + 2 middle)
    while (wait(NULL) > 0)
        ;
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Pipe fails at middle command 4 (5th pipe() call) - last middle before final cmd
static int test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_4(void)
{
    printf("Test: test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_4\n");
    t_shell *sh;
    t_list *cmds;
    pid_t last_pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = create_6_cmd_pipeline();
    
    // Fail on 5th pipe() call (for fourth middle command)
    setup_pipe_mock(5);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return -1 on pipe failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'pipe'", sh->last_err_op, PIPE_OP);
    
    teardown_pipe_mock();
    
    // Wait for children (first + 3 middle)
    while (wait(NULL) > 0)
        ;
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// ============================================================================
// Fork failure tests
// ============================================================================

// Fork fails at first command
static int test_run_pipeline_6_cmds_fork_fails_at_first_cmd(void)
{
    printf("Test: test_run_pipeline_6_cmds_fork_fails_at_first_cmd\n");
    t_shell *sh;
    t_list *cmds;
    pid_t last_pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = create_6_cmd_pipeline();
    
    // Fail on first fork() call
    g_fork_mock.call_count = 0;
    g_fork_mock.fail_at = 1;
    syswrap_set_fork((t_fork_fn)fork_mock_nth);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return -1 on fork failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    mu_assert_intcmp("last_errno should be EAGAIN", sh->last_errno, EAGAIN);
    
    syswrap_set_fork(NULL);
    
    // No children should have been created
    int wait_result = wait(NULL);
    mu_assert("no children should exist", wait_result == -1 && errno == ECHILD);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Fork fails at middle command 1 (2nd fork)
static int test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_1(void)
{
    printf("Test: test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_1\n");
    t_shell *sh;
    t_list *cmds;
    pid_t last_pid;
    int children_reaped = 0;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = create_6_cmd_pipeline();
    
    // Fail on 2nd fork() call (first middle command)
    g_fork_mock.call_count = 0;
    g_fork_mock.fail_at = 2;
    syswrap_set_fork((t_fork_fn)fork_mock_nth);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return -1 on fork failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    
    syswrap_set_fork(NULL);
    
    // Wait for 1 child (first command)
    while (wait(NULL) > 0)
        children_reaped++;
    
    mu_assert_intcmp("should have reaped 1 child", children_reaped, 1);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Fork fails at middle command 2 (3rd fork)
static int test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_2(void)
{
    printf("Test: test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_2\n");
    t_shell *sh;
    t_list *cmds;
    pid_t last_pid;
    int children_reaped = 0;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = create_6_cmd_pipeline();
    
    // Fail on 3rd fork() call (second middle command)
    g_fork_mock.call_count = 0;
    g_fork_mock.fail_at = 3;
    syswrap_set_fork((t_fork_fn)fork_mock_nth);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return -1 on fork failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    
    syswrap_set_fork(NULL);
    
    // Wait for 2 children (first + first middle)
    while (wait(NULL) > 0)
        children_reaped++;
    
    mu_assert_intcmp("should have reaped 2 children", children_reaped, 2);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Fork fails at middle command 3 (4th fork)
static int test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_3(void)
{
    printf("Test: test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_3\n");
    t_shell *sh;
    t_list *cmds;
    pid_t last_pid;
    int children_reaped = 0;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = create_6_cmd_pipeline();
    
    // Fail on 4th fork() call (third middle command)
    g_fork_mock.call_count = 0;
    g_fork_mock.fail_at = 4;
    syswrap_set_fork((t_fork_fn)fork_mock_nth);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return -1 on fork failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    
    syswrap_set_fork(NULL);
    
    // Wait for 3 children (first + 2 middle)
    while (wait(NULL) > 0)
        children_reaped++;
    
    mu_assert_intcmp("should have reaped 3 children", children_reaped, 3);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Fork fails at middle command 4 (5th fork) - last middle
static int test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_4(void)
{
    printf("Test: test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_4\n");
    t_shell *sh;
    t_list *cmds;
    pid_t last_pid;
    int children_reaped = 0;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = create_6_cmd_pipeline();
    
    // Fail on 5th fork() call (fourth middle command)
    g_fork_mock.call_count = 0;
    g_fork_mock.fail_at = 5;
    syswrap_set_fork((t_fork_fn)fork_mock_nth);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return -1 on fork failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    
    syswrap_set_fork(NULL);
    
    // Wait for 4 children (first + 3 middle)
    while (wait(NULL) > 0)
        children_reaped++;
    
    mu_assert_intcmp("should have reaped 4 children", children_reaped, 4);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Fork fails at last command (6th fork)
static int test_run_pipeline_6_cmds_fork_fails_at_last_cmd(void)
{
    printf("Test: test_run_pipeline_6_cmds_fork_fails_at_last_cmd\n");
    t_shell *sh;
    t_list *cmds;
    pid_t last_pid;
    int children_reaped = 0;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    cmds = create_6_cmd_pipeline();
    
    // Fail on 6th fork() call (last command)
    g_fork_mock.call_count = 0;
    g_fork_mock.fail_at = 6;
    syswrap_set_fork((t_fork_fn)fork_mock_nth);
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return -1 on fork failure", result, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    
    syswrap_set_fork(NULL);
    
    // Wait for 5 children (first + 4 middle)
    while (wait(NULL) > 0)
        children_reaped++;
    
    mu_assert_intcmp("should have reaped 5 children", children_reaped, 5);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// ============================================================================
// Execve failure tests for 6-command pipeline
// ============================================================================

// Execve fails at first command (exit 127)
static int test_run_pipeline_6_cmds_execve_fails_at_first_cmd_127(void)
{
    printf("Test: test_run_pipeline_6_cmds_execve_fails_at_first_cmd_127\n");
    t_shell *sh;
    t_list *cmds = NULL;
    pid_t last_pid;
    const char *out_file = "tests/unit/mock-files/pipeline_6_execve_fail.txt";
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    unlink(out_file);
    sh = create_test_shell(test_env, 0);
    
    // cmd1: nonexistent command (will fail with 127)
    const char *argv1[] = {"nonexistent_first_cmd_xyz", "hello world", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    
    // cmd2-5: cat
    for (int i = 0; i < 4; i++)
    {
        const char *argv[] = {"cat", NULL};
        t_cmd *cmd = new_cmd_from_args(argv, 1);
        ft_lstadd_back(&cmds, ft_lstnew(cmd));
    }
    
    // cmd6: wc -c > outfile
    const char *argv6[] = {"wc", "-c", NULL};
    t_cmd *cmd6 = new_cmd_from_args(argv6, 2);
    t_redir *redir = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    cmd6->redirs = ft_lstnew(redir);
    ft_lstadd_back(&cmds, ft_lstnew(cmd6));
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return 0 (fork succeeded)", result, 0);
    
    // Wait for all children
    int statuses[6];
    for (int i = 0; i < 6; i++)
        wait(&statuses[i]);
    
    // At least one should exit with 127
    int found_127 = 0;
    for (int i = 0; i < 6; i++)
    {
        if (WIFEXITED(statuses[i]) && WEXITSTATUS(statuses[i]) == 127)
            found_127 = 1;
    }
    mu_assert("first command should exit with 127", found_127);
    
    unlink(out_file);
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Execve fails at middle command (exit 127)
static int test_run_pipeline_6_cmds_execve_fails_at_middle_cmd_127(void)
{
    printf("Test: test_run_pipeline_6_cmds_execve_fails_at_middle_cmd_127\n");
    t_shell *sh;
    t_list *cmds = NULL;
    pid_t last_pid;
    const char *out_file = "tests/unit/mock-files/pipeline_6_execve_mid_fail.txt";
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    unlink(out_file);
    sh = create_test_shell(test_env, 0);
    
    // cmd1: echo
    const char *argv1[] = {"echo", "hello world", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    
    // cmd2: cat
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    
    // cmd3: nonexistent (will fail with 127)
    const char *argv3[] = {"nonexistent_middle_cmd_xyz", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    
    // cmd4-5: cat
    for (int i = 0; i < 2; i++)
    {
        const char *argv[] = {"cat", NULL};
        t_cmd *cmd = new_cmd_from_args(argv, 1);
        ft_lstadd_back(&cmds, ft_lstnew(cmd));
    }
    
    // cmd6: wc -c > outfile
    const char *argv6[] = {"wc", "-c", NULL};
    t_cmd *cmd6 = new_cmd_from_args(argv6, 2);
    t_redir *redir = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    cmd6->redirs = ft_lstnew(redir);
    ft_lstadd_back(&cmds, ft_lstnew(cmd6));
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return 0 (fork succeeded)", result, 0);
    
    // Wait for all children
    int statuses[6];
    for (int i = 0; i < 6; i++)
        wait(&statuses[i]);
    
    // At least one should exit with 127
    int found_127 = 0;
    for (int i = 0; i < 6; i++)
    {
        if (WIFEXITED(statuses[i]) && WEXITSTATUS(statuses[i]) == 127)
            found_127 = 1;
    }
    mu_assert("middle command should exit with 127", found_127);
    
    unlink(out_file);
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}

// Execve fails at last command (exit 127)
static int test_run_pipeline_6_cmds_execve_fails_at_last_cmd_127(void)
{
    printf("Test: test_run_pipeline_6_cmds_execve_fails_at_last_cmd_127\n");
    t_shell *sh;
    t_list *cmds = NULL;
    pid_t last_pid;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    
    // cmd1: echo
    const char *argv1[] = {"echo", "hello world", NULL};
    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    
    // cmd2-5: cat
    for (int i = 0; i < 4; i++)
    {
        const char *argv[] = {"cat", NULL};
        t_cmd *cmd = new_cmd_from_args(argv, 1);
        ft_lstadd_back(&cmds, ft_lstnew(cmd));
    }
    
    // cmd6: nonexistent (will fail with 127)
    const char *argv6[] = {"nonexistent_last_cmd_xyz", NULL};
    t_cmd *cmd6 = new_cmd_from_args(argv6, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd6));
    
    int result = run_pipeline(sh, cmds, 6, &last_pid);
    
    mu_assert_intcmp("run_pipeline should return 0 (fork succeeded)", result, 0);
    
    // Wait for all children
    int statuses[6];
    for (int i = 0; i < 6; i++)
        wait(&statuses[i]);
    
    // At least one should exit with 127
    int found_127 = 0;
    for (int i = 0; i < 6; i++)
    {
        if (WIFEXITED(statuses[i]) && WEXITSTATUS(statuses[i]) == 127)
            found_127 = 1;
    }
    mu_assert("last command should exit with 127", found_127);
    
    free_cmds(cmds);
    free_shell(sh);
    return (0);
}



int main(void)
{
    printf("=== Unit Tests for exec_pipeline ===\n\n");

    printf("\n--- FIRST do_first_command tests ---\n");
    mu_run_test(test_do_first_command_happy_path_no_redir);
    mu_run_test(test_do_first_command_happy_path_with_redir_in);
    mu_run_test(test_do_first_command_happy_path_with_redir_out);
    mu_run_test(test_do_first_command_happy_path_with_both_redirs);
    mu_run_test(test_do_first_command_fork_failure);
    mu_run_test(test_do_first_command_open_failure_redir_in);
    mu_run_test(test_do_first_command_open_failure_redir_out_mock);
    mu_run_test(test_do_first_command_dup2_failure_first_call);
    mu_run_test(test_do_first_command_dup2_failure_second_call);
    mu_run_test(test_do_first_command_execve_failure_126);
    mu_run_test(test_do_first_command_execve_failure_127);
    mu_run_test(test_do_first_command_access_failure_path_resolution);

    printf("\n--- MIDDLE do_middle_command tests ---\n");
    mu_run_test(test_do_middle_command_happy_path_no_redir);
    mu_run_test(test_do_middle_command_happy_path_with_redir_in);
    mu_run_test(test_do_middle_command_happy_path_with_redir_out);
    mu_run_test(test_do_middle_command_happy_path_with_both_redirs);
    mu_run_test(test_do_middle_command_fork_failure);
    mu_run_test(test_do_middle_command_open_failure_redir_in);
    mu_run_test(test_do_middle_command_open_failure_redir_out_mock);
    mu_run_test(test_do_middle_command_dup2_failure_first_call);
    mu_run_test(test_do_middle_command_dup2_failure_second_call);
    mu_run_test(test_do_middle_command_execve_failure_126);
    mu_run_test(test_do_middle_command_execve_failure_127);
    mu_run_test(test_do_middle_command_access_failure_path_resolution);

    printf("\n--- recursive 4 commands do_middle_commands tests ---\n");
    mu_run_test(test_do_middle_command_recursive_happy_path_three_commands);
    mu_run_test(test_do_middle_command_recursive_fork_failure_on_second_call);
    mu_run_test(test_do_middle_command_recursive_fork_failure_on_third_call);
    mu_run_test(test_do_middle_command_recursive_open_failure_redir_in_on_second_call);
    mu_run_test(test_do_middle_command_recursive_open_failure_redir_out_on_third_call);
    mu_run_test(test_do_middle_command_recursive_dup2_failure_sixth_call);
    mu_run_test(test_do_middle_command_recursive_dup2_failure_fourth_call);
    mu_run_test(test_do_middle_command_recursive_execve_failure_on_third_call_126);
    mu_run_test(test_do_middle_command_recursive_execve_failure_on_third_call_127);

    printf("\n--- LAST do_last_command tests ---\n");

    mu_run_test(test_do_last_command_happy_path_no_redir);
    mu_run_test(test_do_last_command_happy_path_with_redir_in);
    mu_run_test(test_do_last_command_happy_path_with_redir_out);
    mu_run_test(test_do_last_command_happy_path_with_both_redirs);
    mu_run_test(test_do_last_command_fork_failure);
    mu_run_test(test_do_last_command_open_failure_redir_in);
    mu_run_test(test_do_last_command_open_failure_redir_out_mock);
    mu_run_test(test_do_last_command_dup2_failure_first_call);
    mu_run_test(test_do_last_command_dup2_failure_second_call);
    mu_run_test(test_do_last_command_execve_failure_126);
    mu_run_test(test_do_last_command_execve_failure_127);
    mu_run_test(test_do_last_command_access_failure_path_resolution);

    printf("\n--- do_last_command as final stage (two-stage pipeline) ---\n");
    mu_run_test(test_run_pipeline_last_of_two_happy_path);
    mu_run_test(test_run_pipeline_last_of_two_with_redir_on_last);
    mu_run_test(test_run_pipeline_last_of_two_fork_failure_on_last);
    mu_run_test(test_run_pipeline_last_of_two_execve_failure_on_last_126);
    mu_run_test(test_run_pipeline_last_of_two_execve_failure_on_last_127);
    mu_run_test(test_run_pipeline_last_of_two_dup2_failure_on_last);

    printf("\n--- do_last_command as final stage (three-stage pipeline) ---\n");
    mu_run_test(test_run_pipeline_last_of_three_happy_path);
    mu_run_test(test_run_pipeline_last_of_three_with_redir_on_last);
    mu_run_test(test_run_pipeline_last_of_three_fork_failure_on_last);
    mu_run_test(test_run_pipeline_last_of_three_execve_failure_on_last_126);
    mu_run_test(test_run_pipeline_last_of_three_execve_failure_on_last_127);
    mu_run_test(test_run_pipeline_last_of_three_dup2_failure_on_last);

    printf("\n--- Integration-like tests ---\n");
    mu_run_test(test_run_pipeline_last_of_three_intermediate_failures_affect_last);

    printf("\n--- Tests with 6 cmds ---\n");
    // mu_run_test(test_run_pipeline_6_cmds_pipe_happy_path); // misleading fds on valgrind logs
    mu_run_test(test_run_pipeline_6_cmds_pipe_happy_path_redir_out);

    printf("\n--- Tests with 6 cmds - UNHAPPY PATHS ---\n");
    
    printf("\n  -- Pipe failures --\n");
    mu_run_test(test_run_pipeline_6_cmds_pipe_fails_at_first_cmd);
    mu_run_test(test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_1);
    mu_run_test(test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_2);
    mu_run_test(test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_3);
    mu_run_test(test_run_pipeline_6_cmds_pipe_fails_at_middle_cmd_4);
    
    printf("\n  -- Fork failures --\n");
    mu_run_test(test_run_pipeline_6_cmds_fork_fails_at_first_cmd);
    mu_run_test(test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_1);
    mu_run_test(test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_2);
    mu_run_test(test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_3);
    mu_run_test(test_run_pipeline_6_cmds_fork_fails_at_middle_cmd_4);
    mu_run_test(test_run_pipeline_6_cmds_fork_fails_at_last_cmd);
    
    printf("\n  -- Execve failures (127) --\n");
    mu_run_test(test_run_pipeline_6_cmds_execve_fails_at_first_cmd_127);
    mu_run_test(test_run_pipeline_6_cmds_execve_fails_at_middle_cmd_127);
    mu_run_test(test_run_pipeline_6_cmds_execve_fails_at_last_cmd_127);

    mu_summary();
    return 0;
}