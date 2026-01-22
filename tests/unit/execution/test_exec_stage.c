#include "../../../include/minishell.h"
#include "../../../include/exec.h"
#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

volatile sig_atomic_t exit_status = 0;


void	safe_close_p(int *p);
char	*msh_path_from_cmdname(char *arg, t_list *env, t_shell *sh, int *acc_ret);

// ============================================================================
// Generic access mock generator
// ============================================================================

typedef struct s_access_mock {
    int (*access_func)(const char *, int);
    int call_count;
    int fail_at;
} t_access_mock;

static t_access_mock g_access_mock = {NULL, 0, 0};

static int access_mock(const char *path, int mode)
{
    (void)path;
    (void)mode;
    g_access_mock.call_count++;
    if (g_access_mock.call_count == g_access_mock.fail_at)
        return (-1);
    return (0);
}


static void setup_access_mock(int fail_at)
{
    g_access_mock.access_func = access_mock;
    g_access_mock.call_count = 0;
    g_access_mock.fail_at = fail_at;
}

static void teardown_access_mock(void)
{
    g_access_mock.access_func = NULL;
    g_access_mock.call_count = 0;
    g_access_mock.fail_at = 0;
}

static int test_msh_path_from_cmd_name_returns_0_on_not_found_command(void)
{
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
    char *path = msh_path_from_cmdname("not_found", sh->env, sh, &acc_ret);

    mu_assert("path should be null", path == NULL);
    mu_assert_intcmp("acc_ret should be 0", acc_ret, 0);

    teardown_access_mock();
    free_shell(sh);
    return (0);
}

static int test_msh_path_from_cmd_name_returns_0_on_not_found_path(void)
{
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

// TESTS PASSES BUT HAS LEAKS
static int test_msh_path_from_cmd_name_returns_found_valid_first_in_list(void)
{
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
    char *path = msh_path_from_cmdname("ls", sh->env, sh, &acc_ret);

    mu_assert("path should not be null", path != NULL);
    mu_assert_intcmp("acc_ret should be 0", acc_ret, 0);

    teardown_access_mock();
    free_shell(sh);
    return (0);
}

static int test_msh_path_from_cmd_name_returns_found_valid_middle_in_list(void)
{
    return (0);
}


static int test_msh_path_from_cmd_name_returns_found_valid_last_in_list(void)
{
    return (0);
}


int main(void)
{
    // msh_path_from_cmd_name
    /*
    Qué queremos probar: 
        - Que si fa*/
    mu_run_test(test_msh_path_from_cmd_name_returns_0_on_not_found_command);
    mu_run_test(test_msh_path_from_cmd_name_returns_0_on_not_found_path);
    mu_run_test(test_msh_path_from_cmd_name_returns_found_valid_first_in_list);
    mu_run_test(test_msh_path_from_cmd_name_returns_found_valid_middle_in_list);
    mu_run_test(test_msh_path_from_cmd_name_returns_found_valid_last_in_list);
    mu_summary();
    return 0;
}