#include "../../../include/minishell.h"
#include "../../../include/exec.h"
#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

int exit_builtin(char **argv, int parent, int last_status, int *should_exit);
int open_temp_file(void);
char *read_temp_file(int fd);

/*
saalarco@c1r9s6:~/Dev/minishell$ bash
saalarco@c1r9s6:~/Dev/minishell$ exit 2033
exit
saalarco@c1r9s6:~/Dev/minishell$ echo $?
241
saalarco@c1r9s6:~/Dev/minishell$ expr 2033 % 256
241
saalarco@c1r9s6:~/Dev/minishell$ ps
    PID TTY          TIME CMD
2669497 pts/5    00:00:00 zsh
2686597 pts/5    00:00:00 bash
2877209 pts/5    00:00:00 bash
2889299 pts/5    00:00:00 ps
saalarco@c1r9s6:~/Dev/minishell$ 
exit
(.env) saalarco@c1r9s6:~/Dev/minishell/test-sigpipe$ ps
    PID TTY          TIME CMD
2669497 pts/5    00:00:00 zsh
2686597 pts/5    00:00:00 bash
2889362 pts/5    00:00:00 ps
(.env) saalarco@c1r9s6:~/Dev/minishell/test-sigpipe$ 
exit
c1r9s6% ps
    PID TTY          TIME CMD
2669497 pts/5    00:00:00 zsh
2889373 pts/5    00:00:00 ps
c1r9s6% bash
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ exit a
exit
bash: exit: a: numeric argument required
c1r9s6% ps
    PID TTY          TIME CMD
2669497 pts/5    00:00:00 zsh
2889458 pts/5    00:00:00 ps
c1r9s6% bash
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ exit a
exit
bash: exit: a: numeric argument required
c1r9s6% echo $?
2
c1r9s6% ps
    PID TTY          TIME CMD
2669497 pts/5    00:00:00 zsh
2889623 pts/5    00:00:00 ps
c1r9s6% bash
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ exit a b c 
exit
bash: exit: a: numeric argument required
c1r9s6% echo $?
2
c1r9s6% ps
    PID TTY          TIME CMD
2669497 pts/5    00:00:00 zsh
2889698 pts/5    00:00:00 ps
c1r9s6% bash
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ exit 4 3 2 1
exit
bash: exit: too many arguments
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ echo $?
1
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ ps
    PID TTY          TIME CMD
2669497 pts/5    00:00:00 zsh
2889719 pts/5    00:00:00 bash
2889791 pts/5    00:00:00 ps
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ ps
    PID TTY          TIME CMD
2669497 pts/5    00:00:00 zsh
2889719 pts/5    00:00:00 bash
2890141 pts/5    00:00:00 ps
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ ps | exit 33
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ echo $?
33
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ ps
    PID TTY          TIME CMD
2669497 pts/5    00:00:00 zsh
2889719 pts/5    00:00:00 bash
2890266 pts/5    00:00:00 ps
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ ps | exit 33 | echo hola
hola
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ echo $?
0
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ 


*/

// Proposals: outline each test's intent, setup, action, and assertions.
// Use exit_builtin(argv, parent, last_status, &should_exit)
// Parent: do not terminate process; Child: emulate forked child behavior.
// Capture output by dup2 to a temp file when needed to assert printed "exit" or errors.

// 1) test_just_exit_from_parent_prints_exit
// - Args: {"exit", NULL}
// - parent=1, last_status arbitrary (e.g., 0)
// - Expect: exit_builtin prints "exit\n" to stderr (bash prints 'exit') and returns last_status, should_exit=1.
// - Steps: redirect STDERR_FILENO to temp file; call; check file contains "exit\n"; assert return == 0; assert should_exit==1.

// 2) test_just_exit_from_child_prints_exit
// - Args: {"exit", NULL}
// - parent=0 (child), last_status arbitrary
// - Expect: prints "exit\n"; returns last_status; should_exit=1.
// - Steps identical to parent version; only parent flag changes.

// 3) test_from_parent_on_more_than_one_argument_first_is_a_number
// - Args: {"exit", "4", "3", NULL}
// - parent=1
// - Bash: "too many arguments" -> status 1, do not exit (should_exit=0), still prints "exit\n" then error to stderr.
// - Expect: return == 1; should_exit==0; output contains "exit\n" and "too many arguments".
// - Steps: redirect stderr; call; assert contents; assert flags.

// 4) test_from_child_on_more_than_one_argument_first_is_a_number
// - Args: {"exit", "4", "3", NULL}
// - parent=0
// - Bash in pipeline keeps shell running but child exits with 1.
// - Expect: return == 1; should_exit==1; output contains "exit\n" and "too many arguments".
// - Steps: redirect stderr; call; assert.

// 5) test_from_parent_on_more_than_one_argument_first_is_not_a_number
// - Args: {"exit", "a", "b", NULL}
// - parent=1
// - Bash: "numeric argument required" -> status 2, should_exit=1 (parent shell exits), prints "exit\n" and error.
// - Expect: return == 2; should_exit==1; stderr has both messages.

// 6) test_from_child_on_more_than_one_argument_first_is_not_a_number
// - Args: {"exit", "a", "b", NULL}
// - parent=0
// - Expect: return == 2; should_exit==1; stderr messages present.

// 7) test_from_parent_just_one_argument
// - Args: {"exit", "2033", NULL}
// - parent=1
// - Bash: exit with 2033 % 256 == 241; prints "exit\n"; should_exit==1.
// - Expect: return == 241; should_exit==1; stderr contains "exit\n".

// 8) test_from_child_just_one_argument
// - Args: {"exit", "33", NULL}
// - parent=0
// - Expect: return == 33; should_exit==1; stderr "exit\n".

// Skeleton implementations (replace temp file helpers with your existing ones)

static int test_just_exit_from_parent_prints_exit(void)
{
    int fd = open_temp_file();
    int saved_err = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);

    int should_exit = 0;
    char *argv[] = {"exit", NULL};
    int ret = exit_builtin(argv, 1, 0, &should_exit);

    dup2(saved_err, STDERR_FILENO);
    close(saved_err);

    char *out = read_temp_file(fd);
    mu_assert("should print 'exit\\n'", strstr(out, "exit") != NULL);
    mu_assert_intcmp("return last_status (0)", ret, 0);
    mu_assert_intcmp("should_exit==1", should_exit, 1);

    free(out);
    close(fd);
    return 0;
}

static int test_just_exit_from_child_prints_exit(void)
{
    int fd = open_temp_file();
    int saved_err = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);

    int should_exit = 0;
    char *argv[] = {"exit", NULL};
    int ret = exit_builtin(argv, 0, 0, &should_exit);

    dup2(saved_err, STDERR_FILENO);
    close(saved_err);

    char *out = read_temp_file(fd);
    mu_assert("shouldn't print 'exit\\n'", strstr(out, "exit") == NULL);
    mu_assert_intcmp("return last_status (0)", ret, 0);
    mu_assert_intcmp("should_exit==1", should_exit, 1);

    free(out);
    close(fd);
    return 0;
}

static int test_from_parent_on_more_than_one_argument_first_is_a_number(void)
{
    int fd = open_temp_file();
    int saved_err = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);

    int should_exit = 0;
    char *argv[] = {"exit", "4", "3", NULL};
    int ret = exit_builtin(argv, 1, 0, &should_exit);

    dup2(saved_err, STDERR_FILENO);
    close(saved_err);

    char *out = read_temp_file(fd);
    mu_assert("prints 'exit\\n'", strstr(out, "exit") != NULL);
    mu_assert("prints 'too many arguments'", strstr(out, "too many arguments") != NULL);
    mu_assert_intcmp("return 1", ret, 1);
    mu_assert_intcmp("should_exit==0 (parent stays)", should_exit, 0);

    free(out);
    close(fd);
    return 0;
}

static int test_from_child_on_more_than_one_argument_first_is_a_number(void)
{
    int fd = open_temp_file();
    int saved_err = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);

    int should_exit = 0;
    char *argv[] = {"exit", "4", "3", NULL};
    int ret = exit_builtin(argv, 0, 0, &should_exit);

    dup2(saved_err, STDERR_FILENO);
    close(saved_err);

    char *out = read_temp_file(fd);
    mu_assert("prints 'exit\\n'", strstr(out, "exit") != NULL);
    mu_assert("prints 'too many arguments'", strstr(out, "too many arguments") != NULL);
    mu_assert_intcmp("return 1", ret, 1);
    mu_assert_intcmp("should_exit==0", should_exit, 0);

    free(out);
    close(fd);
    return 0;
}

static int test_from_parent_on_more_than_one_argument_first_is_not_a_number(void)
{
    int fd = open_temp_file();
    int saved_err = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);

    int should_exit = 0;
    char *argv[] = {"exit", "a", "b", NULL};
    int ret = exit_builtin(argv, 1, 0, &should_exit);

    dup2(saved_err, STDERR_FILENO);
    close(saved_err);

    char *out = read_temp_file(fd);
    mu_assert("prints 'exit\\n'", strstr(out, "exit") != NULL);
    mu_assert("prints 'numeric argument required'", strstr(out, "numeric argument required") != NULL);
    mu_assert_intcmp("return 2", ret, 2);
    mu_assert_intcmp("should_exit==1", should_exit, 1);

    free(out);
    close(fd);
    return 0;
}

static int test_from_child_on_more_than_one_argument_first_is_not_a_number(void)
{
    int fd = open_temp_file();
    int saved_err = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);

    int should_exit = 0;
    char *argv[] = {"exit", "a", "b", NULL};
    int ret = exit_builtin(argv, 0, 0, &should_exit);

    dup2(saved_err, STDERR_FILENO);
    close(saved_err);

    char *out = read_temp_file(fd);
    mu_assert("prints 'exit\\n'", strstr(out, "exit") != NULL);
    mu_assert("prints 'numeric argument required'", strstr(out, "numeric argument required") != NULL);
    mu_assert_intcmp("return 2", ret, 2);
    mu_assert_intcmp("should_exit==1", should_exit, 1);

    free(out);
    close(fd);
    return 0;
}

static int test_from_parent_just_one_argument(void)
{
    int fd = open_temp_file();
    int saved_err = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);

    int should_exit = 0;
    char *argv[] = {"exit", "2033", NULL};
    int ret = exit_builtin(argv, 1, 0, &should_exit);

    dup2(saved_err, STDERR_FILENO);
    close(saved_err);

    char *out = read_temp_file(fd);
    mu_assert("prints 'exit\\n'", strstr(out, "exit") != NULL);
    mu_assert_intcmp("return 2033 % 256 == 241", ret, 241);
    mu_assert_intcmp("should_exit==1", should_exit, 1);

    free(out);
    close(fd);
    return 0;
}

static int test_from_child_just_one_argument(void)
{
    int fd = open_temp_file();
    int saved_err = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);

    int should_exit = 0;
    char *argv[] = {"exit", "33", NULL};
    int ret = exit_builtin(argv, 0, 0, &should_exit);

    dup2(saved_err, STDERR_FILENO);
    close(saved_err);

    char *out = read_temp_file(fd);
    mu_assert("quiet, doesnt print 'exit\\n'", strstr(out, "exit") == NULL);
    mu_assert_intcmp("return 33", ret, 33);
    mu_assert_intcmp("should_exit==1", should_exit, 1);

    free(out);
    close(fd);
    return 0;
}


int main(void)
{
    mu_run_test(test_just_exit_from_parent_prints_exit);
    mu_run_test(test_just_exit_from_child_prints_exit);
    mu_run_test(test_from_parent_on_more_than_one_argument_first_is_a_number);
    mu_run_test(test_from_child_on_more_than_one_argument_first_is_a_number);
    mu_run_test(test_from_parent_on_more_than_one_argument_first_is_not_a_number);
    mu_run_test(test_from_child_on_more_than_one_argument_first_is_not_a_number);
    mu_run_test(test_from_parent_just_one_argument);
    mu_run_test(test_from_child_just_one_argument);

    mu_summary();
    return 0;
}