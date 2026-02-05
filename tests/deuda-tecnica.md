# Probema de file descriptors extra en el child

Cuando aislo este test: static int test_msh_exec_simple_external_with_args(void)
y corro el ejecutable con valgrind tal que: 
```bash
valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes --child-silent-after-fork=no --track-fds=yes --track-fds=all
```

Me encuentro con este log: 


```log
==2374964== Command: /usr/bin/test -d /tmp
==2374964== Parent PID: 2374963
==2374964== 
==2374964== 
==2374964== FILE DESCRIPTORS: 7 open (1 std) at exit.
==2374964== Open file descriptor 8: /home/saalarco/Dev/minishell/tests/unit/execution/bin/valgrind-fds-results/valgrind_2374964_test_exec_simple.log
==2374964==    <inherited from parent>
==2374964== 
==2374964== Open file descriptor 7: /dev/pts/1
==2374964==    <inherited from parent>
==2374964== 
==2374964== Open file descriptor 6: /dev/pts/1
==2374964==    <inherited from parent>
==2374964== 
==2374964== Open file descriptor 5: /dev/pts/1
==2374964==    <inherited from parent>
==2374964== 
==2374964== Open file descriptor 4: /home/saalarco/Dev/minishell/tests/unit/execution/bin/valgrind-fds-results/valgrind_2374964_test_exec_simple.log
==2374964==    <inherited from parent>
==2374964== 
==2374964== Open file descriptor 3: /home/saalarco/Dev/minishell/tests/unit/execution/bin/valgrind-fds-results/valgrind_2374963_test_exec_simple.log
==2374964==    <inherited from parent>
==2374964== 
==2374964== Open file descriptor 0: /dev/pts/1
==2374964==    <inherited from parent>
==2374964== 
==2374964== 
==2374964== HEAP SUMMARY:
==2374964==     in use at exit: 0 bytes in 0 blocks
==2374964==   total heap usage: 2 allocs, 2 frees, 44 bytes allocated
```
Lo que lleva a pensar en dos problemas: 
    - Hay un file descriptor extra abierto cuando el child termina (0, 5, 6, 7)
    - He cerrado standard file descriptors dentro del child: "... (1 std) at exit."

Esto es un problema pero ya he visto como solucionarlo. 

¿qué he decidido que no es un problema?
Tener MENOS file descriptors en un child process antes del exit. Si voy a hacer exit el proceso padre tiene sus file descriptors intactos.


1. Before execve (your child setup):
       fd 0 → pipe_read (stdin from previous stage)
       fd 1 → pipe_write (stdout to next stage) OR file
       fd 2 → /dev/pts/0 (stderr, unchanged)
       fd 4 → valgrind log (inherited)
       fd 5 → valgrind log (inherited)
       fd 6 → valgrind log (inherited)

2. execve("/usr/bin/head", ...) runs
       head reads from fd 0 (pipe) - WORKS!
       head writes to fd 1 (pipe/file) - WORKS!
       head may write errors to fd 2 - WORKS!

3. head finishes, pipes close:
       fd 0 → CLOSED (pipe EOF, head closed it)
       fd 1 → CLOSED (head closed after writing)
       fd 2 → CLOSED (process ending)
       fd 4 → valgrind log (still open)
       fd 5 → valgrind log (still open)
       fd 6 → valgrind log (still open)

4. Valgrind reports "at exit":
       "0 std" because stdin/stdout/stderr were pipes that closed
       4 fds open: 3, 4, 5, 6


## El tema del SIGPIPE

El caso es que en shell también aparece sigpipe cuando se hace un uso indebido de los pipes. Ejemplo: 

```c
echo "yes | head -n 1" > script.sh
chmod 777 script.sh
valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes --child-silent-after-fork=no --track-origins=yes ./script.sh
```

El resutlado es algo tal que: 
y
==2684270== 
==2684270== Process terminating with default action of signal 13 (SIGPIPE)
==2684270==    at 0x4987907: write (write.c:26)
==2684270==    by 0x10A7AF: ??? (in /usr/bin/yes)
==2684270==    by 0x489CD8F: (below main) (libc_start_call_main.h:58)
==2684270== 
==2684270== HEAP SUMMARY:
==2684270==     in use at exit: 12,252 bytes in 30 blocks
==2684270==   total heap usage: 31 allocs, 1 frees, 12,257 bytes allocated


Y pasa lo mismo con un builtin como 'echo' en vez de 'head' pasa lo mismo, entonces, sí, un SIGPIPE que me aparece es estandar bash behaviour. Lo importante es que no levanta ningún error, ni informa de ninguna forma de que esto ha pasado.



## Si libero p en los builtins existosos obtengo SIGPIPES, si no lo libero, el extremo que no se usa del pipe en uso queda dangling.

No tengo claro si es un problema o no, en todo caso son restaurados correctamente en el parent. 

Test: builtin | builtin | builtin (all ok)
2026-02-05 13:13:00 [msh_exec_pipeline] (last_status: 0, last_err_op: none): Entry point, COMMANDS:

[No redirections]->argv[0]: echo   argv[1]: hello   
[No redirections]->argv[0]: echo   argv[1]: world   
[No redirections]->argv[0]: echo   argv[1]: done   
2026-02-05 13:13:00 [exec_cmds]: Executing pipeline with more than 1 stage

FIRST
[PID 2823543] 🔥[builtin_orq.c]builtin_stage_exit🔥  fd 0 -> /dev/pts/5
[PID 2823543] 🔥[builtin_orq.c]builtin_stage_exit🔥  fd 1 -> pipe:[14024915]
[PID 2823543] 🔥[builtin_orq.c]builtin_stage_exit🔥  fd 2 -> /dev/pts/9
[PID 2823543] 🔥[builtin_orq.c]builtin_stage_exit🔥  fd 3 -> pipe:[14024915]

MIDDLE
[PID 2823545] 🔥[builtin_orq.c]builtin_stage_exit🔥  fd 0 -> pipe:[14024915]
[PID 2823545] 🔥[builtin_orq.c]builtin_stage_exit🔥  fd 1 -> pipe:[14024916]
[PID 2823545] 🔥[builtin_orq.c]builtin_stage_exit🔥  fd 2 -> /dev/pts/9

LAST
[PID 2823546] 🔥[builtin_orq.c]builtin_stage_exit🔥  fd 0 -> pipe:[14024916]
[PID 2823546] 🔥[builtin_orq.c]builtin_stage_exit🔥  fd 1 -> /dev/pts/8
[PID 2823546] 🔥[builtin_orq.c]builtin_stage_exit🔥  fd 2 -> /dev/pts/9

