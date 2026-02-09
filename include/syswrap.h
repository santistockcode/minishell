/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   syswrap.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 18:06:46 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/09 21:31:08 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Syscall wrapper layer allowing injection/failure control */
#ifndef SYSWRAP_H
# define SYSWRAP_H

# include <fcntl.h>
# include <sys/types.h>
# include <unistd.h>

void			clear_saved_fds(t_shell *sh);

/* Wrapper prototypes used across the backend (exec) code */
int				pipe_wrap(int p[2]);
pid_t			fork_wrap(void);
int				dup_wrap(int oldfd);
int				dup2_wrap(int oldfd, int newfd);
int				open_wrap(const char *path, int oflag, ...);
int				close_wrap(int fd);
ssize_t			read_wrap(int fd, void *buf, size_t count);
ssize_t			write_wrap(int fd, const void *buf, size_t count);
int				access_wrap(const char *path, int mode);
pid_t			wait_wrap(int *wstatus);
pid_t			waitpid_wrap(pid_t pid, int *wstatus, int options);
int				chdir_wrap(const char *path);
char			*getcwd_wrap(char *buf, size_t size);
int				unlink_wrap(const char *path);
int				execve_wrap(t_shell *sh, const char *path, char *const argv[],
					char *const envp[]);
char			*readline_wrap(const char *prompt);

/* Optional setters to override behavior (tests can inject stubs) */
typedef int		(*t_pipe_fn)(int p[2]);
typedef pid_t	(*t_fork_fn)(void);
typedef int		(*t_dup_fn)(int);
typedef int		(*t_dup2_fn)(int, int);
typedef int		(*t_open_fn)(const char *path, int oflag, ...);
typedef int		(*t_close_fn)(int);
typedef ssize_t	(*t_read_fn)(int, void *, size_t);
typedef ssize_t	(*t_write_fn)(int, const void *, size_t);
typedef int		(*t_access_fn)(const char *, int);
typedef pid_t	(*t_wait_fn)(int *);
typedef pid_t	(*t_waitpid_fn)(pid_t, int *, int);
typedef int		(*t_chdir_fn)(const char *);
typedef char	*(*t_getcwd_fn)(char *, size_t);
typedef int		(*t_unlink_fn)(const char *);
typedef int		(*t_execve_fn)(const char *path, char *const argv[],
			char *const envp[]);
typedef char	*(*t_readline_fn)(const char *prompt);

void			syswrap_set_pipe(t_pipe_fn fn);
void			syswrap_set_fork(t_fork_fn fn);
void			syswrap_set_dup(t_dup_fn fn);
void			syswrap_set_dup2(t_dup2_fn fn);
void			syswrap_set_open(t_open_fn fn);
void			syswrap_set_close(t_close_fn fn);
void			syswrap_set_read(t_read_fn fn);
void			syswrap_set_write(t_write_fn fn);
void			syswrap_set_access(t_access_fn fn);
void			syswrap_set_wait(t_wait_fn fn);
void			syswrap_set_waitpid(t_waitpid_fn fn);
void			syswrap_set_chdir(t_chdir_fn fn);
void			syswrap_set_getcwd(t_getcwd_fn fn);
void			syswrap_set_unlink(t_unlink_fn fn);
void			syswrap_set_execve(t_execve_fn fn);
void			syswrap_set_readline(t_readline_fn fn);

#endif /* SYSWRAP_H */