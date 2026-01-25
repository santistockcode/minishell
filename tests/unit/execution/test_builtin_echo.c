#include "../../../include/minishell.h"
#include "../../../include/exec.h"
#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

int open_temp_file(void)
{
    int fd = open("tests/unit/mock-files/tmp_helper_builtins.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1)
        perror("open");
    return fd;
}

char *read_temp_file(int fd)
{
    lseek(fd, 0, SEEK_SET);
    char buf[1024];
    ssize_t n = read(fd, buf, sizeof(buf)-1);
    if (n < 0) n = 0;
    buf[n] = '\0';
    return strdup(buf);
}

static int test_echo_no_flags_prints_correctly(void)
{
    int fd = open_temp_file();
    int saved_stdout = dup(STDOUT_FILENO);
    
    char *args[] = {"echo", "hello", "world", NULL};
    echo(args, fd);
    
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    
    char *output = read_temp_file(fd);
    mu_assert("echo without flags should print args with newline", 
              strcmp(output, "hello world\n") == 0);
    
    free(output);
    close(fd);
    return (0);
}

static int test_echo_with_n_flag(void)
{
    int fd = open_temp_file();
    int saved_stdout = dup(STDOUT_FILENO);
    
    dup2(fd, STDOUT_FILENO);
    
    char *args[] = {"echo", "-n", "hello", "world", NULL};
    echo(args, fd);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    
    char *output = read_temp_file(fd);
    mu_assert("echo with -n flag should not print newline", 
              strcmp(output, "hello world") == 0);
    
    free(output);
    close(fd);
    return (0);
}

static int test_echo_multiple_n_flags(void)
{
    int fd = open_temp_file();
    int saved_stdout = dup(STDOUT_FILENO);
    
    dup2(fd, STDOUT_FILENO);
    
    char *args[] = {"echo", "-n", "-n", "-n", "test", NULL};
    echo(args, fd);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    
    char *output = read_temp_file(fd);
    mu_assert("echo with multiple -n flags should not print newline", 
              strcmp(output, "test") == 0);
    
    free(output);
    close(fd);
    return (0);
}

static int test_echo_combined_n_flags(void)
{
    int fd = open_temp_file();
    int saved_stdout = dup(STDOUT_FILENO);
    
    dup2(fd, STDOUT_FILENO);
    
    char *args[] = {"echo", "-nnn", "test", NULL};
    echo(args, fd);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    
    char *output = read_temp_file(fd);
    mu_assert("echo with combined -nnn flag should not print newline", 
              strcmp(output, "test") == 0);
    
    free(output);
    close(fd);
    return (0);
}

static int test_echo_invalid_flag(void)
{
    int fd = open_temp_file();
    int saved_stdout = dup(STDOUT_FILENO);
    
    dup2(fd, STDOUT_FILENO);
    
    char *args[] = {"echo", "-x", "test", NULL};
    echo(args, fd);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    
    char *output = read_temp_file(fd);
    mu_assert("echo with invalid flag should print flag as argument", 
              strcmp(output, "-x test\n") == 0);
    
    free(output);
    close(fd);
    return (0);
}

static int test_echo_empty_args(void)
{
    int fd = open_temp_file();
    int saved_stdout = dup(STDOUT_FILENO);
    
    dup2(fd, STDOUT_FILENO);
    
    char *args[] = {"echo", NULL};
    echo(args, fd);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
    
    char *output = read_temp_file(fd);
    mu_assert("echo without args should print newline only", 
              strcmp(output, "\n") == 0);
    
    free(output);
    close(fd);
    return (0);
}

int main(void)
{
    mu_run_test(test_echo_no_flags_prints_correctly);
    mu_run_test(test_echo_with_n_flag);
    mu_run_test(test_echo_multiple_n_flags);
    mu_run_test(test_echo_combined_n_flags);
    mu_run_test(test_echo_invalid_flag);
    mu_run_test(test_echo_empty_args);

    mu_summary();
    return 0;
}