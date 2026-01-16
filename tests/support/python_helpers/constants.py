"""
Shared constants for backend integration tests.
These mirror C enums and structures from minishell.
"""


class RedirType:
    """Mirror of t_redir_type from C."""
    R_IN = 0
    R_OUT = 1
    R_APPEND = 2
    R_HEREDOC = 3


class ErrorOp:
    """Mirror of error operation types."""
    MALLOC_OP = "malloc"
    OPEN_OP = "open"
    READLINE_OP = "readline"
    FORK_OP = "fork"
    PIPE_OP = "pipe"
    DUP2_OP = "dup2"
    EXECVE_OP = "execve"
    CLOSE_OP = "close"
    WRITE_OP = "write"
    READ_OP = "read"


class ExitStatus:
    """Common exit status codes."""
    SUCCESS = 0
    GENERAL_ERROR = 1
    MISUSE_SHELL_BUILTIN = 2
    CANNOT_EXECUTE = 126
    COMMAND_NOT_FOUND = 127
    FATAL_SIGNAL_BASE = 128
    SIGINT = 128 + 2  # 130
    SIGQUIT = 128 + 3  # 131
    SIGTERM = 128 + 15  # 143


class FileDescriptors:
    """Standard file descriptor constants."""
    STDIN = 0
    STDOUT = 1
    STDERR = 2