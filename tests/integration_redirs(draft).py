# Brainstorming integration tests redirections

# Since fds are inherited, managing redirections is a matter of integration with the shell's execution context.
# By using cpython and FFI we can interact with allowed functions to test leaks, file descriptor states, and other low-level details.
