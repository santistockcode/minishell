#include "../include/exec.h"
#include <errno.h>

/*
Map execve errno to conventional shell exit statuses:
 - 126: Command found but not executable (EACCES/EPERM/EISDIR/ENOEXEC).
 - 127: Command not found (ENOENT).
 - 1:   General failure for other causes.
*/
int msh_status_from_execve_error(int err)
{
    if (err == ENOENT)
        return STATUS_CMD_NOT_FOUND; /* 127 */
    if (err == EACCES || err == EPERM || err == EISDIR || err == ENOEXEC)
        return STATUS_CMD_NOT_EXEC;  /* 126 */
    return 1; /* generic error */
}

/*
Map fork errno to a non-zero shell status.
Common practice is to use 1 for resource errors like EAGAIN/ENOMEM.
*/
int msh_status_from_fork_error(int err)
{
    (void)err; /* currently not differentiating */
    return 1;
}

/*
Map open errno for redirection/setup failures. Bash/zsh typically yield 1
for redirection errors (missing file, permission denied, etc.). We return 1
for all cases to keep behavior consistent. is_outfile kept for possible
future policy differences (truncate vs append), currently unused.
*/
int msh_status_from_open_error(int err, int is_outfile)
{
    (void)err;
    (void)is_outfile;
    return 1;
}
