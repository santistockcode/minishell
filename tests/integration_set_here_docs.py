"""
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

Ejemplo de test super básico: 
Build t_msh_test_redir_spec[] with a sentinel {type=-1}
Build t_msh_test_cmd_spec[] for a pipeline
ctx = msh_test_ctx_create(cmds, n, envp, last_status)
ret = msh_test_set_here_docs(ctx)
Inspect msh_test_get_redir_target(ctx, cmd_i, redir_i) → check for here_doc_* in target
Destroy ctx

Siguientes pasos:
Como exec_cmds y set_here_doc tienen el mismo prototipo no me costará llevarme la implementación a exec_cmds
"""




