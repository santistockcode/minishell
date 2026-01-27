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

// FIXME: problem with existing directory example
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

// ============================================================================
// Tests for edge cases
// ============================================================================

// FIXME: edge case
static int test_msh_exec_simple_null_cmd(void)
{
    printf("Test: msh_exec_simple with NULL cmd\n");
    const char *test_env[] = {"PATH=/usr/bin", NULL};
    t_shell *sh = create_test_shell(test_env, 0);

    int status = msh_exec_simple(sh, NULL, sh->env);

    mu_assert_intcmp("NULL cmd should return 0 (no-op)", status, 0);

    free_shell(sh);
    return 0;
}

// FIXME: edge case
static int test_msh_exec_simple_empty_argv(void)
{
    printf("Test: msh_exec_simple with empty argv\n");
    const char *test_env[] = {"PATH=/usr/bin", NULL};
    t_shell *sh = create_test_shell(test_env, 0);
    t_cmd *cmd = malloc(sizeof(t_cmd));
    cmd->argv = NULL;
    cmd->redirs = NULL;
    cmd->stage_io = NULL;

    int status = msh_exec_simple(sh, cmd, sh->env);

    mu_assert_intcmp("empty argv should return 0 (no-op)", status, 0);

    free(cmd);
    free_shell(sh);
    return 0;
}

// ============================================================================
// Main test runner
// ============================================================================

int main(void)
{
    printf("=== Unit Tests for exec_simple ===\n\n");

    // /* exec_builtin_in_parent tests */
    // mu_run_test(test_exec_builtin_in_parent_export_new_var);
    // mu_run_test(test_exec_builtin_in_parent_export_overwrite_var);
    // mu_run_test(test_exec_builtin_in_parent_export_no_value);
    // mu_run_test(test_exec_builtin_in_parent_unset_existing_var);
    // mu_run_test(test_exec_builtin_in_parent_unset_nonexistent_var);
    // // // // mu_run_test(test_exec_builtin_in_parent_unset_multiple_vars); // bug in unset
    // mu_run_test(test_exec_builtin_in_parent_export_with_output_redir);

    // // // /* msh_exec_simple with external commands */
    // mu_run_test(test_msh_exec_simple_external_cmd_true);
    // mu_run_test(test_msh_exec_simple_external_cmd_false);
    // mu_run_test(test_msh_exec_simple_command_not_found);
    // VOY POR AQUÍ: por que el log dice: 
    mu_run_test(test_msh_exec_simple_external_with_args);

    // /* msh_exec_simple with builtins that modify shell */
    // mu_run_test(test_msh_exec_simple_export_modifies_parent);
    // mu_run_test(test_msh_exec_simple_unset_modifies_parent);

    // // /* msh_exec_simple with redirections */
    // mu_run_test(test_msh_exec_simple_external_with_output_redir);
    // mu_run_test(test_msh_exec_simple_external_with_input_redir);

    /* edge cases */
    // mu_run_test(test_msh_exec_simple_null_cmd); // pending check before calling
    // mu_run_test(test_msh_exec_simple_empty_argv);  // pending check before calling

    printf("\n=== All tests passed! ===\n");
    mu_summary();
    return 0;
}