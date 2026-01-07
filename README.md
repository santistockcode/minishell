# minishell

## Quick start

```bash
git clone <repo url>
git submodule init
git submodule update
```

## Testing strategy (by module)

Effort by test type:
30% Unit
60% Integration  
10% E2E

### Tox - test orchestration

All code to be submitted + debug goes by Makefile + c. All other tests/tools go by tox + python. 
A tox.ini file at root level allows us to maintain code by module on ongoing changes. Expected recipes for e2e, integration and unit. 

### Criterion - Unit tests

Unit tests framework. Criterion runs each test in a separate forked process by default. Very useful for testing functions individually.

### Python script - Integration tests

Candidates for integration tests are those that involve sysemcalls (we can mock system calls and use ctypes in python) and those that involve more than one module. That means we can tests full pipeline execution, error scenarios, edge cases, system call interactions... and so on. 

### Python script + manual tests + external tools - E2e tests

End-to-end tests verify **complete user scenarios**:
- Real-world use cases
- Memory leaks (valgrind)

## Docker workspace

Since we are out of campus and expected to work in different environment a Dockerfile is included under `/tools`folder to mimic (as far as I understood with pipex) a campus computer. 

```bash
docker build -t minishell-dev . # build image

docker volume create minishell-ws # persist workspace

# give permissions to user over workspace
docker run --rm -it \
  --user root \
  -v minishell-ws:/workspace \
  minishell-dev \
  bash -lc 'chown -R 1000:1000 /workspace'

# run container as regular user
docker run --name minishell-dev-container -it --user 1000:1000 \
  --cap-drop=DAC_OVERRIDE --cap-drop=DAC_READ_SEARCH \
  -v minishell-ws:/workspace minishell-dev

# attach terminal as root 
docker run -it --user root -v minishell-ws:/workspace minishell-dev'

```

## Logger

Helper to log execution in `log.h`. Example usage: 
```c
int main(int argc, char** argv)
{
	t_shell *minishell;

	if(argc != 0 && argv[1] != NULL)
		return(0);
	minishell = malloc(sizeof(t_shell));
	minishell->i = 10;
	MSH_LOG("Minishell initialized with i = %d", minishell->i);
	free(minishell);
	return (0);
}
```

## Debugging

How to debug a multiprocess application in container:
1. Already in Dockerfile
```bash
apt-get update
apt-get install gdb
apt-get install -y gdbserver
```

2. Compile with flags -g3 -O0 -DCRITERION_DISABLE_FORKING (if using Criterion)
```bash
./binary-to-run --debug=gdb -debug-transport=tcp:1234 
```

3. From a new terminal (if using a container, new terminal to that container)
```bash
gdb ./binary-to-run 
(gdb) target remote :1234
```
(etc)
```bash
(gdb) set follow-fork-mode child
(gdb) set detach-on-fork off
(gdb) set follow-exec-mode parent 
(gdb) break exec_pipeline
(gdb) run
(and so on)
```

## Spec for expanding variables on here_doc

- Text plain is copied as is. Example: "abc" -> "abc".
- '$' followed by any character except '$' until space, \0 or \n is expanded is possible. Example: "$HOME"->"/user/saalarco".
- '$' followed by any character except '$' until space, \0 or \n is ommited if expansion is not possible. Example: "$HMOE"->"", "$HOME,"->"", "Hi my name is $USR and..."->"Hi my name is  and...".
- '$' followed by space or '$' is kept as literal. Example: "$"->"$", but also "$$"->"$$".
- '$' followed by '?' is expanded for last status code. Example (last status code was 0): "Last status $?"->"Last status 0".

## Readline has leaks, how do I prove it?

- To prove readline has leaks, just comment out setup_readline_mock and teardown_readline_mock on any test in test_set_here_doc and run executable (--keep-binaries flag on python script to keep executables) with valgrind. Compare this with running valgrind against original executable (readline injected) or just read valgrind logs to assert that leaks comes from readline function. 


## Error handling on exec part PRIOR TO EXECUTING PIPELINE (set here doc)

Any syscall error prior to multiprocessing should interrupt pipeline altogether.

### Libft calls (string management)
Pipex isn't protecting string management. Minishell should.

### Malloc and free
Pipex printed something like malloc: strerror(errno) and exited with code '1'. Free was not protected. In minishell we shall not exit. 

### Readline

### Open

### Close

### Unlink

#### Proposed pattern
```c
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "../include/exec.h" // contains t_shell

// call this in the failing low-level function before returning -1
void msh_set_error(t_shell *sh, const char *op, int errnum)
{
    if (!sh)
        return;
    free(sh->last_err_op); // if you allocate previously
    sh->last_err_op = op ? strdup(op) : NULL;
    sh->last_errno = errnum;
}

// top level printing helper
void msh_print_last_error(t_shell *sh, const char *prefix)
{
    const char *op = sh && sh->last_err_op ? sh->last_err_op : prefix;
    int errnum = sh ? sh->last_errno : errno;
    if (op && errnum)
        fprintf(stderr, "minishell: %s: %s\n", op, strerror(errnum));
    else if (op)
        fprintf(stderr, "minishell: %s\n", op);
    else
        fprintf(stderr, "minishell: error\n");
}
```

## Exit spec file

Proposal (pending to check against pipex): 
Internal/fatal (OOM / invariant broken): EXIT_FAILURE (1) — bash behaves like this for many internal failures.
Command redirection/open failure: set last_status = 1 and do not exit the shell (bash reports the error and the command fails).
Command not found: 127
Command found but not executable: 126
SIGINT interrupted job: 130 (You do not need to mirror errno numerically as exit codes.)

### LOG
- 12/10/2025 init team and project in 42 servers