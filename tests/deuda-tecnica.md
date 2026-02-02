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

Sin embargo he debuggeado el test y en ningún momento cierro un file descriptor, por lo que el extra viene de la terminal (de hecho si lo estuviese manejando yo aparecería en valgrind).

EL PROBLEMA es el tema de (1 std) at exit, esto no tiene sentido para mí, valgrind debería identificar 5 y 6 como los std restantes pero no lo hace, y no sé explicar por qué. 


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
