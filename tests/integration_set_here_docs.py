"""
¿Qué queremos probar?

Lo importante de usar ctypes o cffi es que las llamadas al sistema son REALES, y puedo: 
    - testear con una terminal real
    - testear señales reales
    - testear vidas de los proceso reales
    - testear dangling file descriptors

Esto en set_here_doc se traduce en:
    - expansion + env handling
    - heredoc file creation in temp dir (real open/close)
    - error paths (permission denied, etc.)
    - file descriptor management

What to “mock” in integration tests?
    run in a temp directory
    set PATH to a folder containing tiny helper executables you control
    set env vars explicitly

Como exec_cmds y set_here_doc tienen el mismo prototipo no me costará llevarme la implementación a exec_cmds
"""




