#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

#include "../../../include/exec.h"
#include "../../../include/minishell.h"

static int test_set_error_sets_last_err_op(void)
{
    printf("Test: test_set_error_sets_last_err_op\n");
    t_shell *sh = create_test_shell(NULL, 0);
	mu_assert("shell alloc failed", sh != NULL);

	msh_set_error(sh, "open");
	mu_assert("last_err_op not set", sh->last_err_op != NULL);
	mu_assert("last_err_op content mismatch", strcmp(sh->last_err_op, "open") == 0);

	free_shell(sh);
	return 0;
}

static int test_set_error_overwrites_previous(void)
{
    printf("Test: test_set_error_overwrites_previous\n");
	t_shell *sh = create_test_shell(NULL, 0);
	mu_assert("shell alloc failed", sh != NULL);

	msh_set_error(sh, "open");
	mu_assert("first set failed", sh->last_err_op && strcmp(sh->last_err_op, "open") == 0);

	msh_set_error(sh, "read");
	mu_assert("overwrite failed", sh->last_err_op && strcmp(sh->last_err_op, "read") == 0);

	free_shell(sh);
	return 0;
}

static int test_print_last_error_with_errno(void)
{
    printf("Test: test_print_last_error_with_errno\n");
	t_shell *sh = create_test_shell(NULL, 0);
	mu_assert("shell alloc failed", sh != NULL);

	errno = EACCES;
	msh_set_error(sh, "open");

	/* Redirect stderr to a temp file */
	const char *tmp_path = ".tmp_err_exec_errors_1.txt";
	int fd = open(tmp_path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
	mu_assert("failed to open temp file", fd != -1);
	int save_err = dup(STDERR_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);

	msh_print_last_error(sh);

	/* Read back the contents */
	char buf[256] = {0};
	FILE *rf = fopen(tmp_path, "r");
	mu_assert("failed to reopen temp file", rf != NULL);
	fgets(buf, sizeof(buf), rf);
	fclose(rf);
	/* Clean up temp file */
	unlink(tmp_path);

	/* Expected: minishell: open: Permission denied\n */
	mu_assert("stderr message mismatch (with errno)", strstr(buf, "minishell: open: ") == buf);
	mu_assert("missing strerror in message", strstr(buf, "Permission denied") != NULL);

	dup2(save_err, STDERR_FILENO);
	close(save_err);
	free_shell(sh);
	return 0;
}

static int test_print_last_error_without_errno(void)
{
    printf("Test: test_print_last_error_without_errno\n");
	t_shell *sh = create_test_shell(NULL, 0);
	mu_assert("shell alloc failed", sh != NULL);

	errno = 0; /* no errno -> just op */
    msh_set_error(sh, "read");

	const char *tmp_path = ".tmp_err_exec_errors_2.txt";
	int fd = open(tmp_path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
	mu_assert("failed to open temp file", fd != -1);
	int save_err = dup(STDERR_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);

	msh_print_last_error(sh);

	char buf[256] = {0};
	FILE *rf = fopen(tmp_path, "r");
	mu_assert("failed to reopen temp file", rf != NULL);
	fgets(buf, sizeof(buf), rf);
	fclose(rf);
	unlink(tmp_path);

	/* Expected: minishell: read\n */
	mu_assert("stderr message mismatch (without errno)", strcmp(buf, "minishell: read\n") == 0);

	dup2(save_err, STDERR_FILENO);
	close(save_err);
	free_shell(sh);
	return 0;
}

static int test_print_errno_only_when_no_op(void)
{
	printf("Test: test_print_errno_only_when_no_op\n");
	t_shell *sh = create_test_shell(NULL, 0);
	mu_assert("shell alloc failed", sh != NULL);

	/* Ensure no operation is set */
	mu_assert("last_err_op should start NULL", sh->last_err_op == NULL);
	
	const char *tmp_path = ".tmp_err_exec_errors_3.txt";
	int fd = open(tmp_path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
	mu_assert("failed to open temp file", fd != -1);
	int save_err = dup(STDERR_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);
	
	errno = EIO; /* Input/output error */
	msh_print_last_error(sh);

	char buf[256] = {0};
	FILE *rf = fopen(tmp_path, "r");
	mu_assert("failed to reopen temp file", rf != NULL);
	fgets(buf, sizeof(buf), rf);
	fclose(rf);
	unlink(tmp_path);

	/* Expected: minishell: Input/output error\n */
	mu_assert("stderr should start with prefix", strncmp(buf, "minishell: ", 11) == 0);
	mu_assert("missing strerror in message", strstr(buf, "Input/output error") != NULL);

	/* After printing, last_err_op should remain NULL */
	mu_assert("last_err_op should still be NULL", sh->last_err_op == NULL);
	dup2(save_err, STDERR_FILENO);
	close(save_err);

	free_shell(sh);
	return 0;
}

static int test_print_clears_last_err_op(void)
{
	printf("Test: test_print_clears_last_err_op\n");
	t_shell *sh = create_test_shell(NULL, 0);
	mu_assert("shell alloc failed", sh != NULL);

	errno = 0; 
	msh_set_error(sh, "read");
	mu_assert("setup failed", sh->last_err_op && strcmp(sh->last_err_op, "read") == 0);

	const char *tmp_path = ".tmp_err_exec_errors_4.txt";
	int fd = open(tmp_path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
	mu_assert("failed to open temp file", fd != -1);
	int save_err = dup(STDERR_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);

	msh_print_last_error(sh);

	/* After printing, the last_err_op must be cleared */
	mu_assert("last_err_op not cleared after print", sh->last_err_op == NULL);

	/* Consume and cleanup temp file */
	char buf[256] = {0};
	FILE *rf = fopen(tmp_path, "r");
	mu_assert("failed to reopen temp file", rf != NULL);
	fgets(buf, sizeof(buf), rf);
	fclose(rf);
	unlink(tmp_path);
	
	
	/* Expected content check remains consistent */
	mu_assert("stderr message mismatch (without errno)", strcmp(buf, "minishell: read\n") == 0);
	
	dup2(save_err, STDERR_FILENO);
	close(save_err);
	
	free_shell(sh);
	return 0;
}

int main(void)
{
	mu_run_test(test_set_error_sets_last_err_op);
	mu_run_test(test_set_error_overwrites_previous);
	mu_run_test(test_print_last_error_with_errno);
	mu_run_test(test_print_last_error_without_errno);
	mu_run_test(test_print_errno_only_when_no_op);
	mu_run_test(test_print_clears_last_err_op);
	mu_summary();
	return 0;
}
