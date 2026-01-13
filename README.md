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

### Minunit - Unit tests

Execute every `unit_*.py` file undert `/tests`.

```bash
python3 tests/support/scripts/compile_unit_tests.py --run

tox -e unit
tox -e unit-keep
tox -e unit-keep-debug
tox -e unit-valgrind
```

### Tox - integration tests orchestration

All code to be submitted + debug goes by Makefile + c. All other tests/tools go by tox + python. 
A tox.ini file at root level allows us to maintain code by module on ongoing changes. Expected recipes for e2e, integration and unit. 

```bash
pip install -r requirements.txt
tox // run all tests
tox -e integration-backend // run execution integration tests
tox -e integration-frontend // run parsing integration tests
```



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


## Error handling on exec part here_doc (prior to pipeline execution)

Any syscall error prior to multiprocessing should interrupt pipeline altogether, That means top level exec_cmds must print something like: 
"minishell: malloc: <strerror(errno)>", set status code and return (-1)
so that caller knows what to clean.


#### Proposed pattern
```c

/*
typedef struct s_shell
{
	int		i;
	t_list	*env;
	int		last_status;
	int		should_exit;
    char    *last_err_op;

}			t_shell;
*/


// call this in the failing low-level function before returning -1
void msh_set_error(t_shell *sh, const char *op, int saved_errno)
{
    char *tmp_op;

    if (!sh)
        return;
    free(sh->last_err_op);
    tmp_op = ft_strdup(op);
    if (!tmp_op)
        sh->last_err_op = NULL;
    else
        sh->last_err_op = tmp_op;
}

// top level printing helper
void msh_print_last_error(t_shell *sh)
{
    char *op;

    if (sh && sh->last_err_op)
        op = sh->last_err_op;
    if (op && errno)
        fprintf(stderr, "minishell: %s: %s\n", op, strerror(errno));
    else if (op)
        fprintf(stderr, "minishell: %s\n", op);
    else
        fprintf(stderr, "minishell: unknown error\n");
}
```

## TODO: Error handling on execution (after here_doc management)
We use exec_status to get proper status_code. 
If something went wrong in child process that's what exit exists: we exit with correct status_code from the child so the parent sees it. 

### References: 
https://03-jon-perez.gitbook.io/coding-library/c/procesos-e-hilos/estructura-sigaction
https://jesustorres.es/introduccion-a-las-senales-posix

### LOG
- 12/10/2025 init team and project in 42 servers