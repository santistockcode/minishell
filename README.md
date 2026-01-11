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

### References: 
https://03-jon-perez.gitbook.io/coding-library/c/procesos-e-hilos/estructura-sigaction
https://jesustorres.es/introduccion-a-las-senales-posix

### LOG
- 12/10/2025 init team and project in 42 servers