# Unit tests

Makes use of minunit.h

# Integration tests

Because dividing frontend from backend is just a matter of how we divide our work, there's not really a different process for execution_part so just using a ffi to test set_here_docs won't work by using pexpect (mock tty) because we need a terminal for that. 
Test runner heredoc aims to fullfill that layer, serves as an intermediate layer, entry point between pexpect and test_api_set_here_docs,

ACHTUNG!: current integration-backend tox flag fails precisely because the entry point for "execution part" is not set_here_docs but exec_cmds. Later on I will evolve that flag. (FIXME bad smell)

¿Qué queremos probar?
    - expansion + env handling -> unit
    - heredoc file creation in temp dir (real open/close)
    - error paths (permission denied, etc.), errno correctamente seteado por funciones reales
    - file descriptor management
    - signal handling

Lo importante de usar ctypes o cffi es que las llamadas al sistema son REALES, y puedo: 
    - testear con una terminal real (usamos pexpect ya que internamente usamos readline)
    - testear señales reales
    - testear vidas de los procesos reales -> más tarde, en exec_cmds
    - testear dangling file descriptors -> más tarde, en exec_cmds

What to “mock” in integration tests?
    run in a temp directory
    set PATH to a folder containing tiny helper executables you control
    set env vars explicitly

Siguientes pasos:
Como exec_cmds y set_here_doc tienen el mismo prototipo no me costará llevarme la implementación a exec_cmds
"""

# e2e tests

## Test that prompt is displayed
## Test that it accepts commands
## Test output of that command
## Test it accepts signals
## Test mixed commands with wait + signals
## Test here_doc interaction via REPL
## Test shellcheck integration
## Test manual pipex test work here (test pipes)
## Test that output redirection works
## Test that input redirection works
## Test that here-documents are handled correctly
## Test that redirections can be combined
## Test that redirections are restored after command execution
