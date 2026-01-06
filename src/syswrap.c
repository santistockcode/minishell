#include "../include/syswrap.h"
#include "../include/exec.h"
#include "../Libft/include/libft.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>

/*
 * Minimal syscall wrapper indirection. Defaults call the real syscalls,
 * but can be overridden via setter functions. For command injection
 * during tests, malloc_commands() builds a pipeline from env var MSH_CMDS.
 */

static t_pipe_fn    s_pipe_fn    = NULL;
static t_fork_fn    s_fork_fn    = NULL;
static t_dup_fn     s_dup_fn     = NULL;
static t_dup2_fn    s_dup2_fn    = NULL;
static t_open_fn    s_open_fn    = NULL;
static t_close_fn   s_close_fn   = NULL;
static t_read_fn    s_read_fn    = NULL;
static t_write_fn   s_write_fn   = NULL;
static t_access_fn  s_access_fn  = NULL;
static t_wait_fn    s_wait_fn    = NULL;
static t_waitpid_fn s_waitpid_fn = NULL;
static t_chdir_fn   s_chdir_fn   = NULL;
static t_getcwd_fn  s_getcwd_fn  = NULL;
static t_unlink_fn  s_unlink_fn  = NULL;
static t_execve_fn  s_execve_fn  = NULL;
static t_readline_fn s_readline_fn = NULL;

void syswrap_set_pipe(t_pipe_fn fn)     { s_pipe_fn = fn; }
void syswrap_set_fork(t_fork_fn fn)     { s_fork_fn = fn; }
void syswrap_set_dup(t_dup_fn fn)       { s_dup_fn = fn; }
void syswrap_set_dup2(t_dup2_fn fn)     { s_dup2_fn = fn; }
void syswrap_set_open(t_open_fn fn)     { s_open_fn = fn; }
void syswrap_set_close(t_close_fn fn)   { s_close_fn = fn; }
void syswrap_set_read(t_read_fn fn)     { s_read_fn = fn; }
void syswrap_set_write(t_write_fn fn)   { s_write_fn = fn; }
void syswrap_set_access(t_access_fn fn) { s_access_fn = fn; }
void syswrap_set_wait(t_wait_fn fn)     { s_wait_fn = fn; }
void syswrap_set_waitpid(t_waitpid_fn fn){ s_waitpid_fn = fn; }
void syswrap_set_chdir(t_chdir_fn fn)   { s_chdir_fn = fn; }
void syswrap_set_getcwd(t_getcwd_fn fn) { s_getcwd_fn = fn; }
void syswrap_set_unlink(t_unlink_fn fn) { s_unlink_fn = fn; }
void syswrap_set_execve(t_execve_fn fn) { s_execve_fn = fn; }
void syswrap_set_readline(t_readline_fn fn) { s_readline_fn = fn; }

int pipe_wrap(int p[2]) {
	if (s_pipe_fn) return s_pipe_fn(p);
	return pipe(p);
}

pid_t fork_wrap(void) {
	if (s_fork_fn) return s_fork_fn();
	return fork();
}

int dup_wrap(int oldfd) {
	if (s_dup_fn) return s_dup_fn(oldfd);
	return dup(oldfd);
}

int dup2_wrap(int oldfd, int newfd) {
	if (s_dup2_fn) return s_dup2_fn(oldfd, newfd);
	return dup2(oldfd, newfd);
}

int open_wrap(const char *path, int oflag, ...) {
	if (s_open_fn) {
		/* open() has optional mode; tests overriding must ignore extra args */
		return s_open_fn(path, oflag);
	}
	/* Forward to real open with variadic mode for O_CREAT (not used here) */
	return open(path, oflag);
}

int close_wrap(int fd) {
	if (s_close_fn) return s_close_fn(fd);
	return close(fd);
}

ssize_t read_wrap(int fd, void *buf, size_t count) {
	if (s_read_fn) return s_read_fn(fd, buf, count);
	return read(fd, buf, count);
}

ssize_t write_wrap(int fd, const void *buf, size_t count) {
	if (s_write_fn) return s_write_fn(fd, buf, count);
	return write(fd, buf, count);
}

int access_wrap(const char *path, int mode) {
	if (s_access_fn) return s_access_fn(path, mode);
	return access(path, mode);
}

pid_t wait_wrap(int *wstatus) {
	if (s_wait_fn) return s_wait_fn(wstatus);
	return wait(wstatus);
}

pid_t waitpid_wrap(pid_t pid, int *wstatus, int options) {
	if (s_waitpid_fn) return s_waitpid_fn(pid, wstatus, options);
	return waitpid(pid, wstatus, options);
}

int chdir_wrap(const char *path) {
	if (s_chdir_fn) return s_chdir_fn(path);
	return chdir(path);
}

char *getcwd_wrap(char *buf, size_t size) {
	if (s_getcwd_fn) return s_getcwd_fn(buf, size);
	return getcwd(buf, size);
}

int unlink_wrap(const char *path) {
	if (s_unlink_fn) return s_unlink_fn(path);
	return unlink(path);
}

int execve_wrap(const char *path, char *const argv[], char *const envp[]) {
	if (s_execve_fn) return s_execve_fn(path, argv, envp);
	return execve(path, argv, envp);
}

char *readline_wrap(const char *prompt) {
	if (s_readline_fn) return s_readline_fn(prompt);
	return readline(prompt);
}
