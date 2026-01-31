// #include "../include/minishell.h"

// int exec_builtin_in_parent(t_shell *sh, t_cmd *cmd);
// int msh_exec_simple(t_shell *sh, t_cmd *cmd, t_list *env);
#include "../../../include/minishell.h"
#include "../../../include/exec.h"
#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

volatile sig_atomic_t g_exit_status = 0;

/* Function prototypes */
int exec_builtin_in_parent(t_shell *sh, t_cmd *cmd);
int msh_exec_simple(t_shell *sh, t_cmd *cmd, t_list *env);
void safe_close_rd_fds(t_list *redirs);
t_stage_io *prepare_stage_io(t_stage_type pos, t_list *redirs, int in_fd, int *p);


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

/* Helper to find env var in shell env list */
static const char *get_env_value(t_list *env, const char *key)
{
    while (env)
    {
        t_env *e = (t_env *)env->content;
        if (e && e->key && ft_strncmp(e->key, key, ft_strlen(key)) == 0)
            return e->value;
        env = env->next;
    }
    return NULL;
}

/* Helper to check if env var exists */
static int env_has_key(t_list *env, const char *key)
{
    return get_env_value(env, key) != NULL;
}

// ============================================================================
// Tests for exec_builtin_in_parent with EXPORT
// ============================================================================

static int test_exec_builtin_in_parent_export_new_var(void)
{
    printf("Test: exec_builtin_in_parent export new variable\n");
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"export", "MY_VAR=hello", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    int status = exec_builtin_in_parent(sh, cmd);

    mu_assert_intcmp("export should return 0", status, 0);
    mu_assert("MY_VAR should exist in env", env_has_key(sh->env, "MY_VAR"));
    mu_assert("MY_VAR value should be 'hello'",
        ft_strncmp(get_env_value(sh->env, "MY_VAR"), "hello", 5) == 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

static int test_exec_builtin_in_parent_export_overwrite_var(void)
{
    printf("Test: exec_builtin_in_parent export overwrite existing variable\n");
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "EXISTING=old_value",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"export", "EXISTING=new_value", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    int status = exec_builtin_in_parent(sh, cmd);

    mu_assert_intcmp("export should return 0", status, 0);
    mu_assert("EXISTING value should be 'new_value'",
        ft_strncmp(get_env_value(sh->env, "EXISTING"), "new_value", 9) == 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// FIXME: depends on my implementation
static int test_exec_builtin_in_parent_export_no_value(void)
{
    printf("Test: exec_builtin_in_parent export variable without value\n");
    const char *test_env[] = {
        "USER=saalarco",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"export", "EMPTY_VAR", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    int status = exec_builtin_in_parent(sh, cmd);

    mu_assert_intcmp("export should return 0", status, 0);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// ============================================================================
// Tests for exec_builtin_in_parent with UNSET
// ============================================================================

static int test_exec_builtin_in_parent_unset_existing_var(void)
{
    printf("Test: exec_builtin_in_parent unset existing variable\n");
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "TO_DELETE=some_value",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"unset", "TO_DELETE", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    mu_assert("TO_DELETE should exist before unset", env_has_key(sh->env, "TO_DELETE"));

    int status = exec_builtin_in_parent(sh, cmd);

    mu_assert_intcmp("unset should return 0", status, 0);
    mu_assert("TO_DELETE should not exist after unset", !env_has_key(sh->env, "TO_DELETE"));
    mu_assert("USER should still exist", env_has_key(sh->env, "USER"));

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

static int test_exec_builtin_in_parent_unset_nonexistent_var(void)
{
    printf("Test: exec_builtin_in_parent unset nonexistent variable\n");
    const char *test_env[] = {
        "USER=saalarco",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"unset", "DOES_NOT_EXIST", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    int status = exec_builtin_in_parent(sh, cmd);

    mu_assert_intcmp("unset nonexistent var should return 0", status, 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// FIXME: pending bug in unset
static int test_exec_builtin_in_parent_unset_multiple_vars(void)
{
    printf("Test: exec_builtin_in_parent unset multiple variables\n");
    const char *test_env[] = {
        "VAR1=value1",
        "VAR2=value2",
        "VAR3=value3",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"unset", "VAR1", "VAR3", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 3);

    int status = exec_builtin_in_parent(sh, cmd);

    mu_assert_intcmp("unset should return 0", status, 0);
    mu_assert("VAR1 should not exist", !env_has_key(sh->env, "VAR1"));
    mu_assert("VAR2 should still exist", env_has_key(sh->env, "VAR2"));
    mu_assert("VAR3 should not exist", !env_has_key(sh->env, "VAR3"));

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// ============================================================================
// Tests for exec_builtin_in_parent with redirections
// ============================================================================

static int test_exec_builtin_in_parent_export_with_output_redir(void)
{
    printf("Test: exec_builtin_in_parent export with output redirection\n");
    const char *test_env[] = {
        "USER=saalarco",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"export", "REDIR_VAR=value", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    /* Add output redirection to /dev/null */
    t_redir *redir = make_redir(R_OUT_APPEND, "/dev/null", 0, -1);
    cmd->redirs = ft_lstnew(redir);

    int status = exec_builtin_in_parent(sh, cmd);

    mu_assert_intcmp("export with redir should return 0", status, 0);
    mu_assert("REDIR_VAR should exist", env_has_key(sh->env, "REDIR_VAR"));

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// ============================================================================
// Tests for msh_exec_simple with external commands
// ============================================================================

static int test_msh_exec_simple_external_cmd_true(void)
{
    printf("Test: msh_exec_simple external command /bin/true\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"true", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    mu_assert_intcmp("true should return 0", status, 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

static int test_msh_exec_simple_external_cmd_false(void)
{
    printf("Test: msh_exec_simple external command /bin/false\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"false", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    mu_assert_intcmp("false should return 1", status, 1);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// FIXME: problem with child process cmd not found
static int test_msh_exec_simple_command_not_found(void)
{
    printf("Test: msh_exec_simple command not found\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"nonexistent_cmd_12345", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    mu_assert_intcmp("command not found should return 127", status, 127);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

static int test_msh_exec_simple_external_with_args(void)
{
    printf("Test: msh_exec_simple external command with arguments\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    /* test -d /tmp should return 0 (directory exists) */
    const char *argv[] = {"test", "-d", "/tmp", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 3);

    int status = msh_exec_simple(sh, cmd, sh->env);

    mu_assert_intcmp("test -d /tmp should return 0", status, 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// ============================================================================
// Tests for msh_exec_simple with builtins that modify shell state
// ============================================================================

static int test_msh_exec_simple_export_modifies_parent(void)
{
    printf("Test: msh_exec_simple export modifies parent shell env\n");
    const char *test_env[] = {
        "USER=saalarco",
        NULL
    };

    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"export", "SIMPLE_VAR=from_simple", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    int status = msh_exec_simple(sh, cmd, sh->env);

    mu_assert_intcmp("export should return 0", status, 0);
    mu_assert("SIMPLE_VAR should exist in parent env",
        env_has_key(sh->env, "SIMPLE_VAR"));
    mu_assert("SIMPLE_VAR value should be 'from_simple'",
        ft_strncmp(get_env_value(sh->env, "SIMPLE_VAR"), "from_simple", 12) == 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

static int test_msh_exec_simple_unset_modifies_parent(void)
{
    printf("Test: msh_exec_simple unset modifies parent shell env\n");
    const char *test_env[] = {
        "USER=saalarco",
        "TO_UNSET=value",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"unset", "TO_UNSET", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    mu_assert("TO_UNSET should exist before", env_has_key(sh->env, "TO_UNSET"));

    int status = msh_exec_simple(sh, cmd, sh->env);

    mu_assert_intcmp("unset should return 0", status, 0);
    mu_assert("TO_UNSET should not exist after", !env_has_key(sh->env, "TO_UNSET"));

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// ============================================================================
// Tests for msh_exec_simple with redirections
// ============================================================================

// FIXME: I can have my own tmp folder
static int test_msh_exec_simple_external_with_output_redir(void)
{
    printf("Test: msh_exec_simple external command with output redirection\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "hello_redir", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    t_redir *redir = make_redir(1, "tests/unit/mock-files/outfile_test_simple.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);

    int status = msh_exec_simple(sh, cmd, sh->env);

    mu_assert_intcmp("echo with redir should return 0", status, 0);

    int fd = open("tests/unit/mock-files/outfile_test_simple.txt", O_RDONLY);
    mu_assert("output file should exist", fd >= 0);
    if (fd >= 0)
    {
        char buf[64] = {0};
        read(fd, buf, sizeof(buf) - 1);
        close(fd);
        mu_assert("file should contain 'hello_redir'",
            ft_strncmp(buf, "hello_redir", 11) == 0);
    }
    close(fd);
    unlink("tests/unit/mock-files/outfile_test_simple.txt");
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// FIXME: I can have my own tmp folder
static int test_msh_exec_simple_external_with_input_redir(void)
{
    printf("Test: msh_exec_simple external command with input redirection\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);

    /* Create temp input file */
    const char *in_file = "/tmp/test_exec_simple_in.txt";
    int wfd = open(in_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(wfd, "line1\nline2\nline3\n", 18);
    close(wfd);

    /* wc -l < input_file -> should count 3 lines */
    const char *argv[] = {"wc", "-l", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    t_redir *redir = make_redir(R_IN, in_file, 0, -1);
    cmd->redirs = ft_lstnew(redir);

    /* redirect stdout to capture output */
    const char *out_file = "/tmp/test_exec_simple_wc_out.txt";
    t_redir *out_redir = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    ft_lstadd_back(&cmd->redirs, ft_lstnew(out_redir));

    int status = msh_exec_simple(sh, cmd, sh->env);

    mu_assert_intcmp("wc -l should return 0", status, 0);

    /* Verify output contains "3" */
    int fd = open(out_file, O_RDONLY);
    if (fd >= 0)
    {
        char buf[32] = {0};
        read(fd, buf, sizeof(buf) - 1);
        close(fd);
        mu_assert("wc output should contain '3'", ft_strchr(buf, '3') != NULL);
    }
    close(fd);
    unlink(in_file);
    unlink(out_file);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}


// ...existing code...

// ============================================================================
// Tests for msh_exec_simple with FAILING SYSCALLS (external commands)
// ============================================================================

// Access failure during path resolution (permission denied on executable)
static int test_msh_exec_simple_failing_access(void)
{
    printf("Test: msh_exec_simple failing access (permission denied)\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);

    // Make access() always fail with EACCES
    setup_access_mock(1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    teardown_access_mock();

    // Should return 126 (permission denied) or 127 (not found, depending on implementation)
    mu_assert("access failure should return 126 or 127", status == 126 || status == 127);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// Fork failure when executing external command
static int test_msh_exec_simple_failing_fork(void)
{
    printf("Test: msh_exec_simple failing fork\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);

    // Make fork() fail
    syswrap_set_fork((t_fork_fn)fork_fail);

    int status = msh_exec_simple(sh, cmd, sh->env);

    syswrap_set_fork(NULL);

    // Fork failure typically returns -1 or sets an error status
    mu_assert_intcmp("fork failure should return -1", status, -1);
    mu_assert_strcmp("last_err_op should be 'fork'", sh->last_err_op, FORK_OP);
    mu_assert_intcmp("last_errno should be EAGAIN", sh->last_errno, EAGAIN);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// Execve failure (permission denied -> exit 126)
static int test_msh_exec_simple_failing_execve_126(void)
{
    printf("Test: msh_exec_simple failing execve (permission denied, 126)\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);

    setup_execve_mock(1, 126);

    int status = msh_exec_simple(sh, cmd, sh->env);

    teardown_execve_mock();

    mu_assert_intcmp("execve EACCES should return 126", status, 126);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// Execve failure (command not found -> exit 127)
static int test_msh_exec_simple_failing_execve_127(void)
{
    printf("Test: msh_exec_simple failing execve (not found, 127)\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);

    setup_execve_mock(1, 127);

    int status = msh_exec_simple(sh, cmd, sh->env);

    teardown_execve_mock();

    mu_assert_intcmp("execve ENOENT should return 127", status, 127);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// Open failure with input redirection
static int test_msh_exec_simple_failing_open_redir_in(void)
{
    printf("Test: msh_exec_simple failing open (input redirection)\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);

    // Add input redirection to nonexistent file
    t_redir *redir = make_redir(R_IN, "tests/unit/mock-files/nonexistent_simple.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);

    int status = msh_exec_simple(sh, cmd, sh->env);

    // Open failure on redir should return 1
    mu_assert_intcmp("open failure should return 1", status, 1);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// Open failure with output redirection (using mock)
static int test_msh_exec_simple_failing_open_redir_out(void)
{
    printf("Test: msh_exec_simple failing open (output redirection, mock)\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "test", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    // Add output redirection
    t_redir *redir = make_redir(R_OUT_TRUNC, "tests/unit/mock-files/out_simple.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);

    // Make open() fail with EACCES
    setup_open_fails_at_call(open_wrap_eaccess, 0);

    int status = msh_exec_simple(sh, cmd, sh->env);

    syswrap_set_open(NULL);

    mu_assert_intcmp("open failure should return 1", status, 1);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// Dup failure when saving fds
static int test_msh_exec_simple_failing_dup(void)
{
    printf("Test: msh_exec_simple failing dup (save fds)\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "test", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    // Make dup() fail on first call (when saving stdin)
    setup_dup_mock(1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    teardown_dup_mock();

    // Dup failure should return error status (1 or -1 depending on implementation)
    mu_assert("dup failure should return error", status != 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// Dup2 failure when redirecting
static int test_msh_exec_simple_failing_dup2(void)
{
    printf("Test: msh_exec_simple failing dup2 (redirect)\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "test", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    // Add redirection so dup2 is called
    t_redir *redir = make_redir(R_OUT_TRUNC, "/dev/null", 0, -1);
    cmd->redirs = ft_lstnew(redir);

    // Make dup2() fail on first call
    setup_dup2_mock(1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    teardown_dup2_mock();

    mu_assert("dup2 failure should return error", status != 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// ============================================================================
// Tests for msh_exec_simple with FAILING SYSCALLS (builtins)
// ============================================================================

// Dup failure when saving fds for builtin
static int test_msh_exec_simple_failing_dup_builtin(void)
{
    printf("Test: msh_exec_simple failing dup for builtin\n");
    const char *test_env[] = {
        "USER=saalarco",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"export", "TEST_VAR=value", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    // Make dup() fail on first call
    setup_dup_mock(1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    teardown_dup_mock();

    // Should return error but not crash
    mu_assert("dup failure for builtin should return error", status != 0);
    
    // Variable should NOT have been exported (failed before execution)
    mu_assert("TEST_VAR should not exist after dup failure", 
              !env_has_key(sh->env, "TEST_VAR"));

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// Dup2 failure when redirecting for builtin
static int test_msh_exec_simple_failing_dup2_builtin(void)
{
    printf("Test: msh_exec_simple failing dup2 for builtin\n");
    const char *test_env[] = {
        "USER=saalarco",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "builtin_test", NULL};  // echo is a builtin
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    // Add redirection
    t_redir *redir = make_redir(R_OUT_TRUNC, "/dev/null", 0, -1);
    cmd->redirs = ft_lstnew(redir);

    // Make dup2() fail
    setup_dup2_mock(1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    teardown_dup2_mock();

    mu_assert("dup2 failure for builtin should return error", status != 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// Open failure for builtin with redirection
static int test_msh_exec_simple_failing_open_builtin(void)
{
    printf("Test: msh_exec_simple failing open for builtin\n");
    const char *test_env[] = {
        "USER=saalarco",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "test", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    // Add output redirection to a "blocked" file
    t_redir *redir = make_redir(R_OUT_TRUNC, "tests/unit/mock-files/blocked.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);

    // Make open() fail
    setup_open_fails_at_call(open_wrap_eaccess, 0);

    int status = msh_exec_simple(sh, cmd, sh->env);

    syswrap_set_open(NULL);

    mu_assert_intcmp("open failure for builtin should return 1", status, 1);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// Export builtin: verify shell state not modified on dup failure
static int test_msh_exec_simple_export_no_modify_on_dup_failure(void)
{
    printf("Test: msh_exec_simple export does not modify env on dup failure\n");
    const char *test_env[] = {
        "EXISTING=original",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"export", "EXISTING=modified", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    setup_dup_mock(1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    teardown_dup_mock();

    mu_assert("should return error on dup failure", status != 0);
    
    // EXISTING should still have original value
    const char *val = get_env_value(sh->env, "EXISTING");
    mu_assert("EXISTING should still exist", val != NULL);
    if (val)
        mu_assert("EXISTING should have original value", 
                  ft_strncmp(val, "original", 8) == 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// Unset builtin: verify shell state not modified on dup failure
static int test_msh_exec_simple_unset_no_modify_on_dup_failure(void)
{
    printf("Test: msh_exec_simple unset does not modify env on dup failure\n");
    const char *test_env[] = {
        "TO_KEEP=value",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"unset", "TO_KEEP", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    setup_dup_mock(1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    teardown_dup_mock();

    mu_assert("should return error on dup failure", status != 0);
    
    // TO_KEEP should still exist (unset should not have executed)
    mu_assert("TO_KEEP should still exist after failed unset", 
              env_has_key(sh->env, "TO_KEEP"));

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// CD builtin: verify directory not changed on dup failure
static int test_msh_exec_simple_cd_no_change_on_dup_failure(void)
{
    printf("Test: msh_exec_simple cd does not change directory on dup failure\n");
    const char *test_env[] = {
        "HOME=/tmp",
        "PWD=/home",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    
    char original_cwd[PATH_MAX];
    getcwd(original_cwd, PATH_MAX);
    
    const char *argv[] = {"cd", "/tmp", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    setup_dup_mock(1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    teardown_dup_mock();

    char current_cwd[PATH_MAX];
    getcwd(current_cwd, PATH_MAX);

    mu_assert("should return error on dup failure", status != 0);
    mu_assert("directory should not have changed", 
              ft_strncmp(original_cwd, current_cwd, ft_strlen(original_cwd)) == 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

// ============================================================================
// Tests for msh_exec_simple verifying fd restoration after errors
// ============================================================================

static int test_msh_exec_simple_restores_fds_on_open_failure(void)
{
    printf("Test: msh_exec_simple restores fds on open failure\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);

    // Add redirection to nonexistent file
    t_redir *redir = make_redir(R_IN, "tests/unit/mock-files/nonexistent.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);

    // Save original stdin/stdout/stderr
    int orig_stdin = dup(STDIN_FILENO);
    int orig_stdout = dup(STDOUT_FILENO);
    int orig_stderr = dup(STDERR_FILENO);

    int status = msh_exec_simple(sh, cmd, sh->env);

    // Verify fds are still valid and pointing to original destinations
    mu_assert("stdin should be valid after error", fcntl(STDIN_FILENO, F_GETFD) != -1);
    mu_assert("stdout should be valid after error", fcntl(STDOUT_FILENO, F_GETFD) != -1);
    mu_assert("stderr should be valid after error", fcntl(STDERR_FILENO, F_GETFD) != -1);

    mu_assert_intcmp("should return error status", status, 1);

    close(orig_stdin);
    close(orig_stdout);
    close(orig_stderr);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}

static int test_msh_exec_simple_restores_fds_on_dup2_failure(void)
{
    printf("Test: msh_exec_simple restores fds on dup2 failure\n");
    const char *test_env[] = {
        "PATH=/usr/bin:/bin",
        NULL
    };
    t_shell *sh = create_test_shell(test_env, 0);
    const char *argv[] = {"echo", "test", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);

    t_redir *redir = make_redir(R_OUT_TRUNC, "/dev/null", 0, -1);
    cmd->redirs = ft_lstnew(redir);

    setup_dup2_mock(1);

    int status = msh_exec_simple(sh, cmd, sh->env);

    teardown_dup2_mock();

    // Verify standard fds are still valid
    mu_assert("stdin should be valid after dup2 error", fcntl(STDIN_FILENO, F_GETFD) != -1);
    mu_assert("stdout should be valid after dup2 error", fcntl(STDOUT_FILENO, F_GETFD) != -1);
    mu_assert("stderr should be valid after dup2 error", fcntl(STDERR_FILENO, F_GETFD) != -1);

    mu_assert("should return error status", status != 0);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return 0;
}



// ============================================================================
// Main test runner
// ============================================================================

int main(void)
{
    // printf("=== Unit Tests for exec_simple ===\n\n");

    // /* exec_builtin_in_parent tests */
    // mu_run_test(test_exec_builtin_in_parent_export_new_var);
    // mu_run_test(test_exec_builtin_in_parent_export_overwrite_var);
    // mu_run_test(test_exec_builtin_in_parent_export_no_value);
    // mu_run_test(test_exec_builtin_in_parent_unset_existing_var);
    // mu_run_test(test_exec_builtin_in_parent_unset_nonexistent_var);
    // // // // // mu_run_test(test_exec_builtin_in_parent_unset_multiple_vars); // bug in unset
    // mu_run_test(test_exec_builtin_in_parent_export_with_output_redir);

    // // // // /* msh_exec_simple with external commands */
    // mu_run_test(test_msh_exec_simple_external_cmd_true);
    // mu_run_test(test_msh_exec_simple_external_cmd_false);
    // mu_run_test(test_msh_exec_simple_command_not_found);
    // // mu_run_test(test_msh_exec_simple_external_with_args); // deuda técnica

    // // /* msh_exec_simple with builtins that modify shell */
    // mu_run_test(test_msh_exec_simple_export_modifies_parent);
    // mu_run_test(test_msh_exec_simple_unset_modifies_parent);

    // // // /* msh_exec_simple with redirections */
    // mu_run_test(test_msh_exec_simple_external_with_output_redir);
    // mu_run_test(test_msh_exec_simple_external_with_input_redir);

    // printf("\n--- Failing syscalls (external commands) ---\n");
    // mu_run_test(test_msh_exec_simple_failing_access);
    // mu_run_test(test_msh_exec_simple_failing_fork);
    // mu_run_test(test_msh_exec_simple_failing_execve_126);
    // mu_run_test(test_msh_exec_simple_failing_execve_127);
    // mu_run_test(test_msh_exec_simple_failing_open_redir_in);
    // VOY POR AQUÍ (esto hay que refactorizarlo)
    mu_run_test(test_msh_exec_simple_failing_open_redir_out);
    // mu_run_test(test_msh_exec_simple_failing_dup);
    // mu_run_test(test_msh_exec_simple_failing_dup2);

    // printf("\n--- Failing syscalls (builtins) ---\n");
    // mu_run_test(test_msh_exec_simple_failing_dup_builtin);
    // mu_run_test(test_msh_exec_simple_failing_dup2_builtin);
    // mu_run_test(test_msh_exec_simple_failing_open_builtin);
    // mu_run_test(test_msh_exec_simple_export_no_modify_on_dup_failure);
    // mu_run_test(test_msh_exec_simple_unset_no_modify_on_dup_failure);
    // mu_run_test(test_msh_exec_simple_cd_no_change_on_dup_failure);

    // printf("\n--- FD restoration tests ---\n");
    // mu_run_test(test_msh_exec_simple_restores_fds_on_open_failure);
    // mu_run_test(test_msh_exec_simple_restores_fds_on_dup2_failure);

    printf("\n=== All tests passed! ===\n");
    mu_summary();
    return 0;
}