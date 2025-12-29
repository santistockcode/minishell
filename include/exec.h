#ifndef EXEC_H
# define EXEC_H
#include "minishell.h"

typedef enum e_redir_type {
	R_IN,          // <
	R_OUT_TRUNC,   // >
	R_OUT_APPEND,  // >>
	R_HEREDOC      // <<
}   t_redir_type;

/*
** Redirection descriptor
** - type: kind of redirection
** - fd: target stream (usually 0 for stdin, 1 for stdout)
** - target: filename for in/out redirs, or heredoc delimiter
** - quoted: for heredoc; 1 if delimiter was quoted (no expansion), 0 otherwise
*/
typedef struct s_redir {
	t_redir_type      type;
	int               fd;
	char             *target; // path or delimiter for here doc
	int               quoted;
}   t_redir;

// one simple command in a pipeline
/* One simple command in a pipeline
** - argv: NULL-terminated vector (already de-quoted by parser)
** - redirs: list of t_redir* nodes (or NULL)
*/
typedef struct s_cmd {
	char           **argv;
	t_list         *redirs;
}   t_cmd;


/* Backend */
/* Execute a cmds (pipeline list of t_cmd* or unique cmd). Returns the exit status of the last
** command (0 on success), and updates sh->last_status accordingly. */
int	exec_cmds(t_shell *sh, t_list *cmd_first);

/* Free an entire cmds list, including argv strings, redirs and list nodes. */
void	free_cmds(t_list *cmd_first);

/* Execute pipeline when pipelines are involved*/
int msh_exec_pipeline(t_shell *sh, t_list *cmd_first, int nstages);

/* 
** Use an I/O descriptor to avoid 5+ arguments and keep clarity.
*/
typedef enum e_out_mode { OM_PIPE = 0, OM_TRUNC = 1, OM_APPEND = 2 } t_out_mode;

typedef struct s_stage_io {
	int        in_fd;     /* input fd (or -1 if none) */
	int        out_fd;    /* output fd (or -1 if none) */
	t_out_mode out_mode;  /* how the output was opened */
} t_stage_io;

/* Execute a stage in pipeline: applies redirs (already prepared), runs builtin or external. */
int	msh_exec_stage(t_shell *sh, t_cmd *cmd, const t_stage_io *io, t_list *env);

/*Execute simple command (no pipelines involved)*/
int	msh_exec_simple(t_shell *sh, t_cmd *cmd, t_list *env);
/*
Previous function makes use of:
int msh_apply_redirs_parent(t_cmd *cmd, int *save_in, int *save_out); 
and void msh_restore_stdio(int save_in, int save_out);
*/

/* Heredoc */
/* Process all heredocs in the pipeline. On success returns 0 and prepares
** per-command input (e.g. temp file paths). Returns -1 on error. */
int    set_here_doc(t_shell *sh, t_list *cmd_first);

/* Read heredoc from user until delim. If should_expand==1, perform variable
** expansion using sh->env; otherwise keep literal content. Returns a heap
** string with the collected content, or NULL on error. Caller frees. */
char   *fetch_here_doc_from_user(t_shell *sh, const char *delim, int should_expand);

/* Expansion helpers */
/* Expand variables in heredoc content when needed (quoted==0).
** Returns a heap string with expansions applied using env; caller frees. */
char   *msh_expand_heredoc(const char *content, t_list *env);

/* Error and status mapping (execution) */
/* Conventional shell status codes */
#define STATUS_CMD_NOT_EXEC   126   /* permission denied or not executable */
#define STATUS_CMD_NOT_FOUND  127   /* command not found */

/* Map execve errno to a shell exit status (e.g., 126 for EACCES, 127 for ENOENT). */
int    msh_status_from_execve_error(int err);

/* Map open errno to handling status. is_outfile=1 for output file scenarios. */
int    msh_status_from_open_error(int err, int is_outfile);

/* Map fork errno to non-zero status for pipeline abort decisions. */
int    msh_status_from_fork_error(int err);

/* Emit a concise error message for a command (to stderr). */
void   msh_emit_error_cmd(const char *cmd, const char *msg);

/* Emit an errno-based error for an operation on a path (uses strerror/perror). */
void   msh_emit_errno_path(const char *op, const char *path);

/* Exec helpers */
/* Return 1 if name is a builtin, 0 otherwise. */
int     msh_is_builtin(const char *name);

/* Dispatch a builtin by name and argv. Returns the builtin's exit status. */
int     msh_run_builtin(t_shell *sh, const char *name, char **argv, t_list *env);

/* Resolve an external command name using PATH from env.
** Returns a heap-allocated absolute path or NULL if not found. */
char    *msh_resolve_path(const char *name, t_list *env);

/* Execute an external command (fork/exec).
** Returns child exit status or appropriate error code. */
int     msh_exec_external(t_shell *sh, char **argv, t_list *env);

/* Builtins */
int     builtin_echo(t_shell *sh, char **argv);                       /* supports -n */
int     builtin_cd(t_shell *sh, char **argv, t_list *env);            /* relative/absolute */
int     builtin_pwd(t_shell *sh);
int     builtin_export(t_shell *sh, char **argv, t_list *env);        /* no options */
int     builtin_unset(t_shell *sh, char **argv, t_list *env);         /* no options */
int     builtin_env(t_shell *sh, t_list *env);                        /* no args */



#endif