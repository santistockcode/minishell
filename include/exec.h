/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 15:05:40 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/21 18:05:24 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXEC_H
# define EXEC_H

typedef struct s_shell	t_shell;
typedef struct s_list	t_list;

// TODO: Implement pipes and redirections after here_docs

typedef enum e_redir_type
{
	R_IN,
	R_OUT_TRUNC,
	R_OUT_APPEND,
	R_HEREDOC
}					t_redir_type;

/*
** Redirection descriptor
** - type: kind of redirection
** - fd: target stream (usually 0 for stdin, 1 for stdout)
** - target: filename for in/out redirs, or heredoc delimiter
** - quoted: for heredoc; 1 if delimiter was quoted (no expansion), 0 otherwise
*/
typedef struct s_redir
{
	t_redir_type	type;
	int				fd;
	char			*target; // path or delimiter for here doc
	int				quoted;
}					t_redir;


typedef enum e_stage_type
{
	FIRST = 0,
	MIDDLE = 1,
	LAST = 2
}					t_stage_type;


/*
** Use an I/O descriptor to avoid 5+ arguments and keep clarity.
*/
typedef enum e_out_mode
{
	OM_PIPE = 0,
	OM_TRUNC = 1,
	OM_APPEND = 2
}					t_out_mode;


typedef struct s_stage_io
{
	int			in_fd; // input fd (or -1 if none)
	int			out_fd; // output fd (or -1 if none)
	t_out_mode	out_mode; // how the output was opened
}					t_stage_io;

// one simple command in a pipeline
/* One simple command in a pipeline
** - argv: NULL-terminated vector (already de-quoted by parser)
** - redirs: list of t_redir* nodes (or NULL)
*/
typedef struct s_cmd
{
	char			**argv;
	t_list			*redirs;
	t_stage_io		*stage_io; // do not use, never leaves exec part not NULL
}					t_cmd;

/* Backend */

/* Execute a cmds (pipeline list of t_cmd* or unique cmd). 
** Returns the exit status of the last
** command (0 on success), and updates sh->last_status accordingly. 
*/
int					exec_cmds(t_shell *sh, t_list *cmd_first);

/* Free an entire cmds list, including argv strings, redirs and list nodes. */
void				free_cmds(t_list *cmd_first);

/* Execute pipeline when there are pipes*/
int					msh_exec_pipeline(t_shell *sh, t_list *cmd_first,
						int nstages);



/* Execute a stage in pipeline: applies redirs (already prepared),
	runs builtin or external. */
int msh_exec_stage(t_shell *sh, t_cmd *cmd, t_list *env, int *p);


/*Execute simple command (no pipelines involved)*/
int					msh_exec_simple(t_shell *sh, t_cmd *cmd, t_list *env);


// Previous function makes use of:
int		msh_apply_redirs_parent(t_cmd *cmd, int *save_in,
						int *save_out);
void			msh_restore_stdio(int save_in, int save_out);

/* 
** Heredoc (prior to exec commands) 
*/

/* Process all heredocs in the pipeline. On success returns 0 and prepares
** per-command input (e.g. temp file paths). Returns -1 on error. */
int					set_here_docs(t_shell *sh, t_list *cmd_first);

/* Expand variables in heredoc content when needed (quoted==0).
** Returns a heap string with expansions applied using env; caller frees. */
char				*expand_hd(const char *content, t_shell *sh);

// exec utils
int					safe_close(int fd);
int					get_unique_pid_of_process(t_shell *sh);
int					calculate_status_from_errno();


// Unlink all temporary files used for here_docs
void				unlink_hds(t_list *cmds);

// Basic here_doc part (prior to execution) error handling
void				msh_set_error(t_shell *sh, const char *op);
void				msh_print_last_error(t_shell *sh);
int msh_status_from_execve_error(int err);

# define MALLOC_OP "malloc"
# define READLINE_OP "readline"
# define OPEN_OP "open"
# define READ_OP "read"
# define MISSING_FDS_OP "file descriptors"
# define PIPE_OP "pipe"
# define FORK_OP "fork"
# define CLOSE_OP "close"
# define ACCESS_OP "access"
# define EXECVE_OP "execve"
#define STATUS_CMD_NOT_FOUND 127
#define STATUS_CMD_NOT_EXEC 126
# define DUP2_OP "dup2"



// TODO: errors in execution part

/* Error and status mapping (execution) */
/* Conventional shell status codes */
// # define STATUS_CMD_NOT_EXEC 126 /* permission denied or not executable */
// # define STATUS_CMD_NOT_FOUND 127 /* command not found */

// /* Map execve errno to a shell exit status (e.g., 126 for EACCES,
// 	127 for ENOENT). */
// int					msh_status_from_execve_error(int err);

// /* Map open errno to hanlde stat. is_outfile=1 for output file scenarios. */
// int					msh_status_from_open_error(int err, int is_outfile);

// /* Map fork errno to non-zero status for pipeline abort decisions. */
// int					msh_status_from_fork_error(int err);

// /* Emit a concise error message for a command (to stderr). */
// void				msh_emit_error_cmd(const char *cmd, const char *msg);

// /* Emit an errno-based error for an operation on a path (uses strerror)*/
// void				msh_emit_errno_path(const char *op, const char *path);

// /* Exec helpers */
// /* Return 1 if name is a builtin, 0 otherwise. */
// int					msh_is_builtin(const char *name);

// /* Dispatch a builtin by name and argv. Returns the builtin's exit status. */
// int			msh_run_builtin(t_shell *sh, const char *name, char **argv,
// 						t_list *env);

// /* Resolve an external command name using PATH from env.
// ** Returns a heap-allocated absolute path or NULL if not found. */
// char				*msh_resolve_path(const char *name, t_list *env);

// /* Execute an external command (fork/exec).
// ** Returns child exit status or appropriate error code. */
// int			msh_exec_external(t_shell *sh, char **argv, t_list *env);

// /* Builtins */
// /* supports -n */
// int					builtin_echo(t_shell *sh, char **argv);

// /* reative / absolute */
// int					builtin_cd(t_shell *sh, char **argv, t_list *env);

// int					builtin_pwd(t_shell *sh);

// /* no options */
// int					builtin_export(t_shell *sh, char **argv, t_list *env);

// /* no options */
// int					builtin_unset(t_shell *sh, char **argv, t_list *env);

// /* no args */
// int					builtin_env(t_shell *sh, t_list *env);

#endif