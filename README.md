*Este proyecto ha sido creado como parte del currículo de 42 por mnieto-m, saalarco.*


## Description

Minishell is a project that consists of creating a simple command interpreter, inspired by bash. The goal is to understand how processes, redirections, pipes, and signal handling work in a Unix environment.

The shell allows executing basic commands, handling environment variables, implementing input/output redirections, and chaining commands through pipes.

We have divided our work between a "frontend" (parsing) and a "backend" (execution) with special focus on robust testing using python and deep understanding on parsing needs.

## Instructions

### Compilation

```bash
git clone <repo url>
git submodule init
git submodule update
```

```bash
make
```

### Execution

```bash
./minishell
```

### Clean up

```bash
make clean
make fclean
make re
```

## Resources

### Documentation

- [Manual de Bash](https://www.gnu.org/software/bash/manual/)
- [The Open Group Base Specifications - Shell Command Language](https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html)
- Man pages: `fork`, `execve`, `pipe`, `dup2`, `wait`, `signal`
- [Makefile Tutorial](https://makefiletutorial.com/)
- [Estructura sigaction - Coding Library](https://03-jon-perez.gitbook.io/coding-library/c/procesos-e-hilos/estructura-sigaction)
- [Introducción a las señales POSIX](https://jesustorres.es/introduccion-a-las-senales-posix)


### AI usage

- Explanation of concepts (signals, multiprocess programming...).
- Generation of this README.md file.
- Test coverage suggestions in Python.
- Scaffolding for integration tests.
- Automating Valgrind and file descriptor leak shell tests.
- Quick refactoring to comply with linter rules.
- Debugging assistance and error interpretation.

