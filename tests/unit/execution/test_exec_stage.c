#include "../../../include/minishell.h"
#include "../../../include/exec.h"
#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

volatile sig_atomic_t g_exit_status = 0;


void	safe_close_p(int *p);
char	*msh_path_from_cmdname(char *arg, t_list *env, t_shell *sh, int *acc_ret);
char	*msh_resolve_path(char **args, t_list *envp, t_shell *sh);
void	msh_exec_stage(t_shell *sh, t_cmd *cmd, t_list *env, int *p);
t_stage_io  *prepare_stage_io(t_stage_type pos, t_list *redirs, int in_fd, int *p);
int prepare_redirs(t_list *redirs);
int create_mock_pipe_with_data(const char *data);
void safe_close_stage_io(t_stage_io *stage_io);
void	msh_exec_builtin_child(t_shell *sh, t_cmd *cmd, int *p);


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

static int test_msh_path_from_cmd_name_returns_0_on_not_found_command(void)
{
    printf("Test: test_msh_path_from_cmd_name_returns_0_on_not_found_command\n");
    t_shell *sh;
    int acc_ret;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/home/saalarco/Dev/minishell/.env/bin:/home/saalarco/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games",
        "SHELL=/bin/bash",
        NULL
    };


    acc_ret = 0;
    sh = create_test_shell(test_env, 0);
    setup_access_mock(10000);
    char *path = msh_path_from_cmdname("laksdflkjadfslkjafd", sh->env, sh, &acc_ret);

    mu_assert("path should be null", path == NULL);
    mu_assert_intcmp("acc_ret should be 0", acc_ret, 0);

    teardown_access_mock();
    free_shell(sh);
    return (0);
}

static int test_msh_path_from_cmd_name_returns_0_on_not_found_path(void)
{
    printf("Test: test_msh_path_from_cmd_name_returns_0_on_not_found_path\n");
    t_shell *sh;
    int acc_ret;
    const char *test_env[] = {
        "USER=saalarco",
        "SHELL=/bin/bash",
        NULL
    };


    acc_ret = 0;
    sh = create_test_shell(test_env, 0);
    setup_access_mock(10000);
    char *path = msh_path_from_cmdname("cat", sh->env, sh, &acc_ret);

    mu_assert("path should be null", path == NULL);
    mu_assert_intcmp("acc_ret should be 0", acc_ret, 0);

    teardown_access_mock();
    free_shell(sh);
    return (0);
}

static int test_msh_path_from_cmd_name_returns_found_valid_first_in_list(void)
{
    printf("Test: test_msh_path_from_cmd_name_returns_found_valid_first_in_list\n");
    t_shell *sh;
    int acc_ret;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/home/saalarco/Dev/minishell/.env/bin:/home/saalarco/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/sbin:/bin:/usr/games:/usr/local/games",
        "SHELL=/bin/bash",
        NULL
    }; 

    acc_ret = 0;
    sh = create_test_shell(test_env, 0);
    setup_access_mock(10000);
    char *path = msh_path_from_cmdname("ls", sh->env, sh, &acc_ret);

    mu_assert("path should not be null", path != NULL);
    mu_assert_intcmp("acc_ret should be 0", acc_ret, 0);
    logger("[test] found path:", path);
    free(path);
    teardown_access_mock();
    free_shell(sh);
    return (0);
}

static int test_msh_path_from_cmd_name_returns_found_valid_last_in_list(void)
{
    printf("Test: test_msh_path_from_cmd_name_returns_found_valid_last_in_list\n");
    t_shell *sh;
    int acc_ret;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/home/saalarco/Dev/minishell/.env/bin:/home/saalarco/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/sbin:/bin:/usr/games:/usr/local/games:/usr/bin",
        "SHELL=/bin/bash",
        NULL
    }; 

    acc_ret = 0;
    sh = create_test_shell(test_env, 0);
    setup_access_mock(10000);
    char *path = msh_path_from_cmdname("ls", sh->env, sh, &acc_ret);

    mu_assert("path should not be null", path != NULL);
    mu_assert_intcmp("acc_ret should be 0", acc_ret, 0);
    logger("[test] found path:", path);
    free(path);
    teardown_access_mock();
    free_shell(sh);
    return (0);
}


static int test_msh_path_from_cmd_no_leaks_on_access_fails_first_call(void)
{
    printf("Test: test_msh_path_from_cmd_no_leaks_on_access_fails_first_call\n");
    t_shell *sh;
    int acc_ret;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/home/saalarco/Dev/minishell/.env/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/sbin:/bin:/usr/games:/usr/local/games:/usr/bin",
        "SHELL=/bin/bash",
        NULL
    }; 

    acc_ret = 0;
    sh = create_test_shell(test_env, 0);
    setup_access_mock(1);
    char *path = msh_path_from_cmdname("ls", sh->env, sh, &acc_ret);

    mu_assert("path should be null", path == NULL);
    mu_assert_intcmp("acc_ret should be -1", acc_ret, -1);

    teardown_access_mock();
    free_shell(sh);
    return (0);
}

static int test_msh_path_from_cmd_no_leaks_on_access_fails_middle_call(void)
{
    printf("Test: test_msh_path_from_cmd_no_leaks_on_access_fails_middle_call\n");
    t_shell *sh;
    int acc_ret;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/home/saalarco/Dev/minishell/.env/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/sbin:/bin:/usr/games:/usr/local/games:/usr/bin",
        "SHELL=/bin/bash",
        NULL
    }; 

    acc_ret = 0;
    sh = create_test_shell(test_env, 0);
    setup_access_mock(3);
    char *path = msh_path_from_cmdname("ls", sh->env, sh, &acc_ret);

    mu_assert("path should be null", path == NULL);
    mu_assert_intcmp("acc_ret should be -1", acc_ret, -1);

    teardown_access_mock();
    free_shell(sh);

    return (0);
}

static int test_msh_path_from_cmd_no_leaks_on_access_fails_last_call(void)
{
    printf("Test: test_msh_path_from_cmd_no_leaks_on_access_fails_last_call\n");
        t_shell *sh;
    int acc_ret;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/games:/usr/local/games:/usr/bin",
        "SHELL=/bin/bash",
        NULL
    }; 

    acc_ret = 0;
    sh = create_test_shell(test_env, 0);
    setup_access_mock(3);
    char *path = msh_path_from_cmdname("ls", sh->env, sh, &acc_ret);

    mu_assert("path should be null", path == NULL);
    mu_assert_intcmp("acc_ret should be -1", acc_ret, -1);

    teardown_access_mock();
    free_shell(sh);
    return (0);
}

// msh_resolve_path tests

// access fails at 1 call (the direct access to args[0])
static int test_msh_resolve_path_access_fails_at_direct_call(void)
{
    printf("Test: test_msh_resolve_path_access_fails_at_direct_call\n");
    t_shell *sh;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    sh = create_test_shell(test_env, 0);
    setup_access_mock(1); // fail at first access call
    const char *argv[] = {"wc", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    char *path = msh_resolve_path(cmd->argv, sh->env, sh);
    mu_assert("path should be null", path == NULL);
    mu_assert_intcmp("sh->last_errno should be EACCES", sh->last_errno, EACCES);
    mu_assert_strcmp("sh->last_err_op should be 'access'", sh->last_err_op, "access");
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_access_mock();
    return (0);
}

// acess fails at 2 call (the first PATH entry)
static int test_msh_resolve_path_access_fails_at_path_first_call(void)
{
    printf("Test: test_msh_resolve_path_access_fails_at_path_first_call\n");
    t_shell *sh;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    sh = create_test_shell(test_env, 0);
    setup_access_mock(2); // fail at first access call

    const char *argv[] = {"wc", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    char *path = msh_resolve_path(cmd->argv, sh->env, sh);

    mu_assert("path should be null", path == NULL);
    mu_assert_intcmp("sh->last_errno should be EACCES", sh->last_errno, EACCES);
    mu_assert_strcmp("sh->last_err_op should be 'access'", sh->last_err_op, "access");
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_access_mock();
    return (0);
}

// access succeed at 1 call (the direct access to args[0])
static int test_msh_resolve_path_access_succeeds_at_direct_call(void)
{
    printf("Test: test_msh_resolve_path_access_succeeds_at_direct_call\n");
    t_shell *sh;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    sh = create_test_shell(test_env, 0);
    setup_access_mock_happy_path(1); // succeed at first access call

    const char *argv[] = {"/usr/bin/wc", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    char *path = msh_resolve_path(cmd->argv, sh->env, sh);
    mu_assert("path should not be null", path != NULL);
    mu_assert_strcmp("path should be /usr/bin/wc", path, "/usr/bin/wc");
    free(path);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_access_mock();
    return (0);
}

// test happy path of msh_resolve_path returning valid path
static int test_msh_resolve_path_happy_path_found_in_path(void)
{
    printf("Test: test_msh_resolve_path_happy_path_found_in_path\n");
    t_shell *sh;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    sh = create_test_shell(test_env, 0);
    setup_access_mock_happy_path(2); // succeed at second access call

    const char *argv[] = {"wc", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    char *path = msh_resolve_path(cmd->argv, sh->env, sh);
    mu_assert("path should not be null", path != NULL);
    mu_assert_strcmp("path should be /usr/bin/wc", path, "/usr/bin/wc");
    free(path);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_access_mock();
    return (0);
}

// test happy path of msh_resolve_path returning valid path direct args[0]
static int test_msh_resolve_path_happy_path_found_in_args0(void)
{
    printf("Test: test_msh_resolve_path_happy_path_found_in_args0\n");
    t_shell *sh;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    sh = create_test_shell(test_env, 0);
    // setup_access_mock_happy_path(1); // succeed at first access call

    const char *argv[] = {"/usr/bin/ls", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    char *path = msh_resolve_path(cmd->argv, sh->env, sh);
    mu_assert("path should not be null", path != NULL);
    mu_assert_strcmp("path should be /usr/bin/ls", path, "/usr/bin/ls");
    free(path);
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    // teardown_access_mock();
    return (0);
}

// tests msh_exec_stage

// dup2 fails
// cmd FIRST exits on dup2 failure
static int test_first_msh_exec_stage_dup2_failure_first_call(void)
{
    printf("Test: test_msh_exec_stage_dup2_failure_first_call\n");
	int p[2];
    pid_t pid;
    t_shell *sh;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    setup_dup2_mock(1); // fail at first dup2 call
    
    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        // no redirs
        t_stage_io *stage_io = prepare_stage_io(0, NULL, -1, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        // never reaches here
        exit(99);
    }
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);

    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_first_msh_exec_stage_dup2_failure_first_call_with_redir_in(void)
{
    printf("Test: test_first_msh_exec_stage_dup2_failure_first_call_with_redir_in\n");
    // ls -l < infile
    int p[2];
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    t_redir *redir = make_redir(0, "tests/unit/mock-files/infile.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);

    setup_dup2_mock(1);

    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        redirs = cmd->redirs;
        int result_prep_red = prepare_redirs(redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(0, redirs, -1, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        // never reaches here
        exit(99);
    }

    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);

    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_first_msh_exec_stage_dup2_failure_first_call_with_redir_out(void)
{
    printf("Test: test_first_msh_exec_stage_dup2_failure_first_call_with_redir_out\n");
    // ls -l > outfile
    int p[2];
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    t_redir *redir = make_redir(1, "tests/unit/mock-files/outfile.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);

    setup_dup2_mock(1);

    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        redirs = cmd->redirs;
        int result_prep_red = prepare_redirs(redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(0, redirs, -1, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        // never reaches here
        exit(99);
    }

    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);

    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_first_msh_exec_stage_dup2_failure_second_call_with_redir_in_and_out(void)
{
    printf("Test: test_first_msh_exec_stage_dup2_failure_second_call_with_redir_in_and_out\n");
    // ls -l < infile > outfile
    int p[2];
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    t_redir *redir_in = make_redir(0, "tests/unit/mock-files/infile.txt", 0, -1);
    t_redir *redir_out = make_redir(1, "tests/unit/mock-files/outfile.txt", 0, -1);
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_in));
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_out));

    setup_dup2_mock(2);

    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        redirs = cmd->redirs;
        int result_prep_red = prepare_redirs(redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(0, redirs, -1, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        // never reaches here
        exit(99);
    }

    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);

    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_firsta_msh_exec_stage_dup2_failure_first_call_with_here_doc(void)
{
    printf("Test: test_firsta_msh_exec_stage_dup2_failure_first_call_with_here_doc\n");
    // cat << EOF
    int p[2];
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(3, "tests/unit/mock-files/.infile-hd-mock.txt", 1, -1); // here-doc
    cmd->redirs = ft_lstnew(redir);

    setup_dup2_mock(1);

    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        redirs = cmd->redirs;
        int result_prep_red = prepare_redirs(redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(0, redirs, -1, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        // never reaches here
        exit(99);
    }

    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);

    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);

    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

// cmd MIDDLE exits on dup2 failure
// MIDDLE command tests for dup2 failures
static int test_middle_msh_exec_stage_dup2_failure_first_call_no_redir(void)
{
    printf("Test: test_middle_msh_exec_stage_dup2_failure_first_call_no_redir\n");
    // middle cmd: cat (reads from previous pipe, writes to next pipe)
    int p[2];
    int in_fd;
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);

    in_fd = create_mock_pipe_with_data("test data\n");
    mu_assert("in_fd should be valid", in_fd >= 0);

    setup_dup2_mock(1);

    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        redirs = cmd->redirs;
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        prepare_redirs(redirs);
        t_stage_io *stage_io = prepare_stage_io(MIDDLE, redirs, in_fd, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        exit(99);
    }
    
    close(in_fd);
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_middle_msh_exec_stage_dup2_failure_first_call_with_redir_in(void)
{
    printf("Test: test_middle_msh_exec_stage_dup2_failure_first_call_with_redir_in\n");
    // middle cmd: cat < infile (redir overrides previous pipe???)
    int p[2];
    int in_fd;
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(0, "tests/unit/mock-files/infile.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    // Mock previous pipe
    in_fd = create_mock_pipe_with_data("test data, will be overwritten by redir in\n");

    setup_dup2_mock(1);
    
    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        redirs = cmd->redirs;
        int result_prep_red = prepare_redirs(redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(MIDDLE, redirs, in_fd, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        exit(99);
    }
    
    close(in_fd);
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_middle_msh_exec_stage_dup2_failure_first_call_with_redir_out(void)
{
    printf("Test: test_middle_msh_exec_stage_dup2_failure_first_call_with_redir_out\n");
    // middle cmd: cat > outfile (reads from previous pipe, writes to file)
    int p[2];
    int in_fd;
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(1, "tests/unit/mock-files/outfile.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    // Mock previous pipe
    in_fd = create_mock_pipe_with_data("test data\n");
    
    setup_dup2_mock(1);
    
    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        redirs = cmd->redirs;
        int result_prep_red = prepare_redirs(redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(MIDDLE, redirs, in_fd, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        exit(99);
    }
    
    close(in_fd);
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_middle_msh_exec_stage_dup2_failure_second_call_with_redir_in_and_out(void)
{
    printf("Test: test_middle_msh_exec_stage_dup2_failure_second_call_with_redir_in_and_out\n");
    // middle cmd: cat < infile > outfile
    int p[2];
    int in_fd;
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir_in = make_redir(0, "tests/unit/mock-files/infile.txt", 0, -1);
    t_redir *redir_out = make_redir(1, "tests/unit/mock-files/outfile.txt", 0, -1);
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_in));
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_out));
    
    // Mock previous pipe
    int temp_pipe[2];
    pipe(temp_pipe);
    close(temp_pipe[1]);
    in_fd = temp_pipe[0];
    
    setup_dup2_mock(2);
    
    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        redirs = cmd->redirs;
        int result_prep_red = prepare_redirs(redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(MIDDLE, redirs, in_fd, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        exit(99);
    }
    
    close(in_fd);
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_middle_msh_exec_stage_dup2_failure_second_call_no_redir(void)
{
    printf("Test: test_middle_msh_exec_stage_dup2_failure_second_call_no_redir\n");
    // middle cmd: cat (in_fd -> stdout goes to pipe, dup2 fails on second call)
    int p[2];
    int in_fd;
    pid_t pid;
    t_shell *sh;
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
    
    // Mock previous pipe
    int temp_pipe[2];
    pipe(temp_pipe);
    write(temp_pipe[1], "test\n", 5);
    close(temp_pipe[1]);
    in_fd = temp_pipe[0];
    
    setup_dup2_mock(2); // fail at second dup2 call
    
    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        t_stage_io *stage_io = prepare_stage_io(MIDDLE, NULL, in_fd, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        exit(99);
    }
    
    close(in_fd);
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

// // cmd LAST exits on dup2 failure
// LAST command tests for dup2 failures
// cmd LAST exits on dup2 failure
static int test_last_msh_exec_stage_dup2_failure_first_call_no_redir(void)
{
    printf("Test: test_last_msh_exec_stage_dup2_failure_first_call_no_redir\n");
    // last cmd: cat (reads from previous pipe, writes to stdout)
    int in_fd;
    pid_t pid;
    t_shell *sh;
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
    
    // Mock previous pipe
    in_fd = create_mock_pipe_with_data("test data\n");
    mu_assert("in_fd should be valid", in_fd >= 0);
    
    setup_dup2_mock(1);
    
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        t_stage_io *stage_io = prepare_stage_io(LAST, NULL, in_fd, NULL);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, NULL);
        exit(99);
    }
    
    close(in_fd);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_last_msh_exec_stage_dup2_failure_first_call_with_redir_in(void)
{
    printf("Test: test_last_msh_exec_stage_dup2_failure_first_call_with_redir_in\n");
    // last cmd: cat < infile (redir overrides previous pipe)
    int in_fd;
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(0, "tests/unit/mock-files/infile.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    // Mock previous pipe
    in_fd = create_mock_pipe_with_data("test data\n");
    
    setup_dup2_mock(1);
    
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        redirs = cmd->redirs;
        int result_prep_red = prepare_redirs(redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(LAST, redirs, in_fd, NULL);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, NULL);
        exit(99);
    }
    
    close(in_fd);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_last_msh_exec_stage_dup2_failure_first_call_with_redir_out(void)
{
    printf("Test: test_last_msh_exec_stage_dup2_failure_first_call_with_redir_out\n");
    // last cmd: cat > outfile (reads from previous pipe, writes to file)
    int in_fd;
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(1, "tests/unit/mock-files/outfile.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    // Mock previous pipe
    in_fd = create_mock_pipe_with_data("test data\n");
    
    setup_dup2_mock(1);
    
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        redirs = cmd->redirs;
        int result_prep_red = prepare_redirs(redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(LAST, redirs, in_fd, NULL);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, NULL);
        exit(99);
    }
    
    close(in_fd);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_last_msh_exec_stage_dup2_failure_second_call_with_redir_in_and_out(void)
{
    printf("Test: test_last_msh_exec_stage_dup2_failure_second_call_with_redir_in_and_out\n");
    // last cmd: cat < infile > outfile
    int in_fd;
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir_in = make_redir(0, "tests/unit/mock-files/infile.txt", 0, -1);
    t_redir *redir_out = make_redir(1, "tests/unit/mock-files/outfile.txt", 0, -1);
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_in));
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_out));
    
    // Mock previous pipe
    int temp_pipe[2];
    pipe(temp_pipe);
    close(temp_pipe[1]);
    in_fd = temp_pipe[0];
    
    setup_dup2_mock(2);
    
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        redirs = cmd->redirs;
        int result_prep_red = prepare_redirs(redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(LAST, redirs, in_fd, NULL);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, NULL);
        exit(99);
    }
    
    close(in_fd);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

static int test_last_msh_exec_stage_dup2_failure_first_call_with_here_doc(void)
{
    printf("Test: test_last_msh_exec_stage_dup2_failure_first_call_with_here_doc\n");
    // last cmd: cat << EOF
    int in_fd;
    pid_t pid;
    t_shell *sh;
    int status;
    t_list *redirs;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"cat", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 1);
    t_redir *redir = make_redir(3, "tests/unit/mock-files/.infile-hd-mock.txt", 1, -1);
    cmd->redirs = ft_lstnew(redir);
    
    // Mock previous pipe (will be overridden by heredoc)
    int temp_pipe[2];
    pipe(temp_pipe);
    close(temp_pipe[1]);
    in_fd = temp_pipe[0];
    
    setup_dup2_mock(1);
    
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        redirs = cmd->redirs;
        int result_prep_red = prepare_redirs(redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(LAST, redirs, in_fd, NULL);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, NULL);
        exit(99);
    }
    
    close(in_fd);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be EXIT_FAILURE", WEXITSTATUS(status), EXIT_FAILURE);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_dup2_mock();
    return (0);
}

// execve failure (test for 126 and 127)
// execve failure tests (126 and 127 exit codes)

// FIRST command execve failures
static int test_first_msh_exec_stage_execve_failure_126_no_redir(void)
{
    printf("Test: test_first_msh_exec_stage_execve_failure_126_no_redir\n");
    int p[2];
    pid_t pid;
    t_shell *sh;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    setup_execve_mock(1, 126);
    
    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        prepare_redirs(cmd->redirs);
        t_stage_io *stage_io = prepare_stage_io(FIRST, cmd->redirs, -1, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        exit(99);
    }
    
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 126", WEXITSTATUS(status), 126);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_execve_mock();
    return (0);
}

static int test_first_msh_exec_stage_execve_failure_127_no_redir(void)
{
    printf("Test: test_first_msh_exec_stage_execve_failure_127_no_redir\n");
    int p[2];
    pid_t pid;
    t_shell *sh;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    setup_execve_mock(1, 127);
    
    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        t_stage_io *stage_io = prepare_stage_io(FIRST, NULL, -1, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        exit(99);
    }
    
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 127", WEXITSTATUS(status), 127);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_execve_mock();
    return (0);
}

static int test_first_msh_exec_stage_execve_failure_126_with_redir(void)
{
    printf("Test: test_first_msh_exec_stage_execve_failure_126_with_redir\n");
    int p[2];
    pid_t pid;
    t_shell *sh;
    int status;
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"ls", "-l"};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    t_redir *redir = make_redir(1, "tests/unit/mock-files/outfile.txt", 0, -1);
    cmd->redirs = ft_lstnew(redir);
    
    setup_execve_mock(1, 126);
    
    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        int result_prep_red = prepare_redirs(cmd->redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(FIRST, cmd->redirs, -1, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        exit(99);
    }
    
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 126", WEXITSTATUS(status), 126);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_execve_mock();
    return (0);
}

// MIDDLE command execve failures
static int test_middle_msh_exec_stage_execve_failure_126_no_redir(void)
{
    printf("Test: test_middle_msh_exec_stage_execve_failure_126_no_redir\n");
    int p[2];
    int in_fd;
    pid_t pid;
    t_shell *sh;
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
    
    in_fd = create_mock_pipe_with_data("test data\n");
    mu_assert("in_fd should be valid", in_fd >= 0);
    
    setup_execve_mock(1, 126);
    
    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        t_stage_io *stage_io = prepare_stage_io(MIDDLE, NULL, in_fd, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        exit(99);
    }
    
    close(in_fd);
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 126", WEXITSTATUS(status), 126);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_execve_mock();
    return (0);
}

static int test_middle_msh_exec_stage_execve_failure_127_no_redir(void)
{
    printf("Test: test_middle_msh_exec_stage_execve_failure_127_no_redir\n");
    int p[2];
    int in_fd;
    pid_t pid;
    t_shell *sh;
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
    
    in_fd = create_mock_pipe_with_data("test data\n");
    
    setup_execve_mock(1, 127);
    
    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        t_stage_io *stage_io = prepare_stage_io(MIDDLE, NULL, in_fd, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        exit(99);
    }
    
    close(in_fd);
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 127", WEXITSTATUS(status), 127);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_execve_mock();
    return (0);
}

static int test_middle_msh_exec_stage_execve_failure_126_with_redirs(void)
{
    printf("Test: test_middle_msh_exec_stage_execve_failure_126_with_redirs\n");
    int p[2];
    int in_fd;
    pid_t pid;
    t_shell *sh;
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
    t_redir *redir_in = make_redir(0, "tests/unit/mock-files/infile.txt", 0, -1);
    t_redir *redir_out = make_redir(1, "tests/unit/mock-files/outfile.txt", 0, -1);
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_in));
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_out));
    
    in_fd = create_mock_pipe_with_data("test data\n");
    
    setup_execve_mock(1, 126);
    
    pipe(p);
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        int result_prep_red = prepare_redirs(cmd->redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(MIDDLE, cmd->redirs, in_fd, p);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, p);
        exit(99);
    }
    
    close(in_fd);
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 126", WEXITSTATUS(status), 126);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_execve_mock();
    return (0);
}

// LAST command execve failures
static int test_last_msh_exec_stage_execve_failure_126_no_redir(void)
{
    printf("Test: test_last_msh_exec_stage_execve_failure_126_no_redir\n");
    int in_fd;
    pid_t pid;
    t_shell *sh;
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
    
    in_fd = create_mock_pipe_with_data("test data\n");
    
    setup_execve_mock(1, 126);
    
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        t_stage_io *stage_io = prepare_stage_io(LAST, NULL, in_fd, NULL);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, NULL);
        exit(99);
    }
    
    close(in_fd);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 126", WEXITSTATUS(status), 126);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_execve_mock();
    return (0);
}

static int test_last_msh_exec_stage_execve_failure_127_no_redir(void)
{
    printf("Test: test_last_msh_exec_stage_execve_failure_127_no_redir\n");
    int in_fd;
    pid_t pid;
    t_shell *sh;
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
    
    in_fd = create_mock_pipe_with_data("test data\n");
    
    setup_execve_mock(1, 127);
    
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        t_stage_io *stage_io = prepare_stage_io(LAST, NULL, in_fd, NULL);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, NULL);
        exit(99);
    }
    
    close(in_fd);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 127", WEXITSTATUS(status), 127);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_execve_mock();
    return (0);
}

static int test_last_msh_exec_stage_execve_failure_126_with_redirs(void)
{
    printf("Test: test_last_msh_exec_stage_execve_failure_126_with_redirs\n");
    int in_fd;
    pid_t pid;
    t_shell *sh;
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
    t_redir *redir_in = make_redir(0, "tests/unit/mock-files/infile.txt", 0, -1);
    t_redir *redir_out = make_redir(1, "tests/unit/mock-files/outfile.txt", 0, -1);
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_in));
    ft_lstadd_back(&(cmd->redirs), ft_lstnew(redir_out));
    
    in_fd = create_mock_pipe_with_data("test data\n");
    
    setup_execve_mock(1, 126);
    
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        int result_prep_red = prepare_redirs(cmd->redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(LAST, cmd->redirs, in_fd, NULL);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, NULL);
        exit(99);
    }
    
    close(in_fd);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 126", WEXITSTATUS(status), 126);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_execve_mock();
    return (0);
}

static int test_last_msh_exec_stage_execve_failure_127_with_here_doc(void)
{
    printf("Test: test_last_msh_exec_stage_execve_failure_127_with_here_doc\n");
    int in_fd;
    pid_t pid;
    t_shell *sh;
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
    t_redir *redir = make_redir(3, "tests/unit/mock-files/.infile-hd-mock.txt", 1, -1);
    cmd->redirs = ft_lstnew(redir);
    
    int temp_pipe[2];
    pipe(temp_pipe);
    close(temp_pipe[1]);
    in_fd = temp_pipe[0];
    
    setup_execve_mock(1, 127);
    
    pid = fork();
    if (pid == 0)
    {
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        int result_prep_red = prepare_redirs(cmd->redirs);
        mu_assert("prepare_redirs should succeed", result_prep_red == 0);
        t_stage_io *stage_io = prepare_stage_io(LAST, cmd->redirs, in_fd, NULL);
        cmd->stage_io = stage_io;
        msh_exec_stage(sh, cmd, sh->env, NULL);
        exit(99);
    }
    
    close(in_fd);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 127", WEXITSTATUS(status), 127);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    teardown_execve_mock();
    return (0);
}

// msh_exec_builtin_child tests
static int test_msh_exec_builtin_child_export_success(void)
{
    printf("Test: test_msh_exec_builtin_child_export_success\n");
    pid_t pid;
    t_shell *sh;
    int status;
    int in_fd;
    t_list *redirs;
    int p[2];
    const char *test_env[] = {
        "USER=saalarco",
        "PATH=/usr/bin:/bin",
        "SHELL=/bin/bash",
        NULL
    };
    pipe(p);
    errno = 0;
    sh = create_test_shell(test_env, 0);
    const char *argv[] = {"export", "TEST_VAR=test_value", NULL};
    t_cmd *cmd = new_cmd_from_args(argv, 2);
    
    pid = fork();
    if (pid == 0)
    {
        redirs = cmd->redirs;
        msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
        prepare_redirs(redirs);
        t_stage_io *stage_io = prepare_stage_io(FIRST, redirs, -1, p);
        cmd->stage_io = stage_io;
        msh_exec_builtin_child(sh, cmd, p);
        exit(99); // Should never reach here
    }
    close(p[0]);
    close(p[1]);
    waitpid(pid, &status, 0);
    
    mu_assert("child should have exited normally", WIFEXITED(status));
    mu_assert_intcmp("child exit status should be 0", WEXITSTATUS(status), 0);
    
    free_cmds(ft_lstnew(cmd));
    free_shell(sh);
    return (0);
}

int main(void)
{
    // msh_path_from_cmd_name
    mu_run_test(test_msh_path_from_cmd_name_returns_0_on_not_found_command);
    mu_run_test(test_msh_path_from_cmd_name_returns_0_on_not_found_path);
    mu_run_test(test_msh_path_from_cmd_name_returns_found_valid_first_in_list);
    mu_run_test(test_msh_path_from_cmd_name_returns_found_valid_last_in_list);
    mu_run_test(test_msh_path_from_cmd_no_leaks_on_access_fails_first_call);
    mu_run_test(test_msh_path_from_cmd_no_leaks_on_access_fails_middle_call);
    mu_run_test(test_msh_path_from_cmd_no_leaks_on_access_fails_last_call);
    
    // msh_resolve_path
    mu_run_test(test_msh_resolve_path_access_fails_at_direct_call);
    mu_run_test(test_msh_resolve_path_access_fails_at_path_first_call);
    mu_run_test(test_msh_resolve_path_access_succeeds_at_direct_call);
    mu_run_test(test_msh_resolve_path_happy_path_found_in_path);
    mu_run_test(test_msh_resolve_path_happy_path_found_in_args0);

    // msh_exec_stage
    //  dup2 failure
    mu_run_test(test_first_msh_exec_stage_dup2_failure_first_call);
    mu_run_test(test_first_msh_exec_stage_dup2_failure_first_call_with_redir_in);
    mu_run_test(test_first_msh_exec_stage_dup2_failure_first_call_with_redir_out);
    mu_run_test(test_first_msh_exec_stage_dup2_failure_second_call_with_redir_in_and_out);
    mu_run_test(test_firsta_msh_exec_stage_dup2_failure_first_call_with_here_doc);
    mu_run_test(test_middle_msh_exec_stage_dup2_failure_first_call_no_redir);
    mu_run_test(test_middle_msh_exec_stage_dup2_failure_first_call_with_redir_in);
    mu_run_test(test_middle_msh_exec_stage_dup2_failure_first_call_with_redir_out);
    mu_run_test(test_middle_msh_exec_stage_dup2_failure_second_call_with_redir_in_and_out);
    mu_run_test(test_middle_msh_exec_stage_dup2_failure_second_call_no_redir);
    mu_run_test(test_last_msh_exec_stage_dup2_failure_first_call_no_redir);
    mu_run_test(test_last_msh_exec_stage_dup2_failure_first_call_with_redir_in);
    mu_run_test(test_last_msh_exec_stage_dup2_failure_first_call_with_here_doc);
    mu_run_test(test_last_msh_exec_stage_dup2_failure_first_call_with_redir_out);
    mu_run_test(test_last_msh_exec_stage_dup2_failure_second_call_with_redir_in_and_out);
    // execve failure
    mu_run_test(test_first_msh_exec_stage_execve_failure_126_no_redir);
    mu_run_test(test_first_msh_exec_stage_execve_failure_127_no_redir);
    mu_run_test(test_first_msh_exec_stage_execve_failure_126_with_redir);
    mu_run_test(test_middle_msh_exec_stage_execve_failure_126_no_redir);
    mu_run_test(test_middle_msh_exec_stage_execve_failure_127_no_redir);
    mu_run_test(test_middle_msh_exec_stage_execve_failure_126_with_redirs);
    mu_run_test(test_last_msh_exec_stage_execve_failure_126_no_redir);
    mu_run_test(test_last_msh_exec_stage_execve_failure_127_no_redir);
    mu_run_test(test_last_msh_exec_stage_execve_failure_126_with_redirs);
    mu_run_test(test_last_msh_exec_stage_execve_failure_127_with_here_doc);
    
    // // msh_exec_builtin_child export happy path
    mu_run_test(test_msh_exec_builtin_child_export_success);

    // test child process leaks using valgrind
    /*
    # Basic command to check child leaks
valgrind \
  --leak-check=full \
  --show-leak-kinds=all \
  --trace-children=yes \
  --track-fds=yes \
  --track-origins=yes \
  ./tests/unit/execution/bin/test_exec_stage

valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes --child-silent-after-fork=no --track-fds=yes --track-fds=all tests/unit/execution/bin/test_exec_stage


# More detailed (useful for debugging):
valgrind \
  --leak-check=full \
  --show-leak-kinds=all \
  --trace-children=yes \
  --child-silent-after-fork=no \
  --track-fds=yes \
  --track-origins=yes \
  ./tests/unit/execution/bin/test_exec_stage
  */
    mu_summary();
    return 0;
}