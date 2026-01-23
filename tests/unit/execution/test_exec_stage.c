#include "../../../include/minishell.h"
#include "../../../include/exec.h"
#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

volatile sig_atomic_t exit_status = 0;


void	safe_close_p(int *p);
char	*msh_path_from_cmdname(char *arg, t_list *env, t_shell *sh, int *acc_ret);
char	*msh_resolve_path(char **args, t_list *envp, t_shell *sh);
void	msh_exec_stage(t_shell *sh, t_cmd *cmd, t_list *env, int *p);

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

static t_execve_mock g_execve_mock = {0, 0};

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

    // NEXT: 
    // FIXME: should exec_pipeline return status from last commnad? does it?

    // TODO: decide correct example for direct access failure in exec_stage
    // what happens if command is ./my-custom-command.sh but have no access? should iterate over path?

    // TODO: norminette exec_stage

    // msh_exec_stage
    // exits on dup2 failure first call
    // exits on dup2 failure second call
    // exits on execve failure 126
    // exits on execve failure 127

    // test child process leaks

    mu_summary();
    return 0;
}