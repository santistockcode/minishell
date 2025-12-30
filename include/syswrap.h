/* Syscall wrapper layer allowing injection/failure control for integration tests */
#ifndef SYSWRAP_H
#define SYSWRAP_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "minishell.h"

/* Wrapper prototypes used across the backend (exec) code */
int     pipe_wrap(int p[2]);
pid_t   fork_wrap(void);
int     dup_wrap(int oldfd);
int     dup2_wrap(int oldfd, int newfd);
int     open_wrap(const char *path, int oflag, ...);
int     close_wrap(int fd);
ssize_t read_wrap(int fd, void *buf, size_t count);
ssize_t write_wrap(int fd, const void *buf, size_t count);
int     access_wrap(const char *path, int mode);
pid_t   wait_wrap(int *wstatus);
pid_t   waitpid_wrap(pid_t pid, int *wstatus, int options);
int     chdir_wrap(const char *path);
char   *getcwd_wrap(char *buf, size_t size);
int     unlink_wrap(const char *path);
int     execve_wrap(const char *path, char *const argv[], char *const envp[]);
char    *readline_wrap(const char *prompt);

/* Optional setters to override behavior (tests can inject stubs) */
typedef int     (*pipe_fn_t)(int p[2]);
typedef pid_t   (*fork_fn_t)(void);
typedef int     (*dup_fn_t)(int);
typedef int     (*dup2_fn_t)(int, int);
typedef int     (*open_fn_t)(const char *path, int oflag, ...);
typedef int     (*close_fn_t)(int);
typedef ssize_t (*read_fn_t)(int, void*, size_t);
typedef ssize_t (*write_fn_t)(int, const void*, size_t);
typedef int     (*access_fn_t)(const char*, int);
typedef pid_t   (*wait_fn_t)(int*);
typedef pid_t   (*waitpid_fn_t)(pid_t, int*, int);
typedef int     (*chdir_fn_t)(const char*);
typedef char*   (*getcwd_fn_t)(char*, size_t);
typedef int     (*unlink_fn_t)(const char*);
typedef int     (*execve_fn_t)(const char *path, char *const argv[], char *const envp[]);
typedef char*   (*readline_fn_t)(const char *prompt);

void    syswrap_set_pipe(pipe_fn_t fn);
void    syswrap_set_fork(fork_fn_t fn);
void    syswrap_set_dup(dup_fn_t fn);
void    syswrap_set_dup2(dup2_fn_t fn);
void    syswrap_set_open(open_fn_t fn);
void    syswrap_set_close(close_fn_t fn);
void    syswrap_set_read(read_fn_t fn);
void    syswrap_set_write(write_fn_t fn);
void    syswrap_set_access(access_fn_t fn);
void    syswrap_set_wait(wait_fn_t fn);
void    syswrap_set_waitpid(waitpid_fn_t fn);
void    syswrap_set_chdir(chdir_fn_t fn);
void    syswrap_set_getcwd(getcwd_fn_t fn);
void    syswrap_set_unlink(unlink_fn_t fn);
void    syswrap_set_execve(execve_fn_t fn);
void    syswrap_set_readline(readline_fn_t fn);

#endif /* SYSWRAP_H */