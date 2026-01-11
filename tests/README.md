# Unit tests

Makes use of minunit.h

# Integration tests
¿Qué queremos probar?
    - expansion + env handling
    - heredoc file creation in temp dir (real open/close)
    - error paths (permission denied, etc.), errno correctamente seteado por funciones reales
    - file descriptor management

Lo importante de usar ctypes o cffi es que las llamadas al sistema son REALES, y puedo: 
    - testear con una terminal real (usamos pexpect ya que internamente usamos readline)
    - testear señales reales
    - testear vidas de los procesos reales
    - testear dangling file descriptors

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
