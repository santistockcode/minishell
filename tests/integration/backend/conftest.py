"""
Shared pytest fixtures for backend integration tests.
"""

import ctypes
import shutil
import pytest
import sys
from pathlib import Path
from typing import List
import pexpect
import os

sys.path.insert(0, str(Path(__file__).parent.parent.parent / "support"))

class TestRedirSpec(ctypes.Structure):
    """Mirrors t_msh_test_redir_spec."""
    _fields_ = [
        ("type", ctypes.c_int),
        ("fd", ctypes.c_int),
        ("target", ctypes.c_char_p),
        ("quoted", ctypes.c_int),
    ]


class TestCmdSpec(ctypes.Structure):
    """Mirrors t_msh_test_cmd_spec."""
    _fields_ = [
        ("argv", ctypes.POINTER(ctypes.c_char_p)),
        ("redirs", ctypes.POINTER(TestRedirSpec)),
    ]


@pytest.fixture(scope="session")
def test_api_lib():
    """Load the compiled test API shared library."""
    project_root = Path(__file__).parent.parent.parent.parent
    lib_path = project_root / "tests" / "integration" / "apis" / "libtest_api_exec.so"
    
    if not lib_path.exists():
        pytest.fail(f"Test API library not found: {lib_path}")
    
    lib = ctypes.CDLL(str(lib_path))
    
    # Define function signatures
    lib.msh_test_ctx_create.argtypes = [
        ctypes.POINTER(TestCmdSpec),
        ctypes.c_int,
        ctypes.POINTER(ctypes.c_char_p),
        ctypes.c_int
    ]
    lib.msh_test_ctx_create.restype = ctypes.c_void_p
    
    lib.msh_test_set_here_docs.argtypes = [ctypes.c_void_p]
    lib.msh_test_set_here_docs.restype = ctypes.c_int
    
    lib.msh_test_get_redir_target.argtypes = [
        ctypes.c_void_p, ctypes.c_int, ctypes.c_int
    ]
    lib.msh_test_get_redir_target.restype = ctypes.c_char_p
    
    lib.msh_test_get_last_err_op.argtypes = [ctypes.c_void_p]
    lib.msh_test_get_last_err_op.restype = ctypes.c_char_p
    
    lib.msh_test_get_last_status.argtypes = [ctypes.c_void_p]
    lib.msh_test_get_last_status.restype = ctypes.c_int
    
    lib.msh_test_ctx_destroy.argtypes = [ctypes.c_void_p]
    lib.msh_test_ctx_destroy.restype = None
    
    return lib


@pytest.fixture
def make_redir_spec():
    """Factory fixture for creating TestRedirSpec objects."""
    def _make(redir_type: int, target: str, quoted: int = 0, fd: int = 0) -> TestRedirSpec:
        return TestRedirSpec(
            type=redir_type,
            fd=fd,
            target=target.encode('utf-8'),
            quoted=quoted
        )
    return _make


@pytest.fixture
def make_sentinel_redir():
    """Factory fixture for creating sentinel redir."""
    def _make() -> TestRedirSpec:
        return TestRedirSpec(type=-1, fd=0, target=None, quoted=0)
    return _make


@pytest.fixture
def make_cmd_spec(make_sentinel_redir):
    """Factory fixture for creating TestCmdSpec objects."""
    def _make(argv: List[str], redirs: List[TestRedirSpec]) -> TestCmdSpec:
        # Create NULL-terminated argv
        argv_arr = (ctypes.c_char_p * (len(argv) + 1))()
        for i, arg in enumerate(argv):
            argv_arr[i] = arg.encode('utf-8')
        argv_arr[len(argv)] = None
        
        # Create sentinel-terminated redirs
        redirs_arr = (TestRedirSpec * (len(redirs) + 1))()
        for i, redir in enumerate(redirs):
            redirs_arr[i] = redir
        redirs_arr[len(redirs)] = make_sentinel_redir()
        
        return TestCmdSpec(argv=argv_arr, redirs=redirs_arr)
    return _make


@pytest.fixture
def make_envp():
    """Factory fixture for creating environment arrays."""
    def _make(env_dict: dict) -> ctypes.Array:
        env_list = [f"{k}={v}" for k, v in env_dict.items()]
        envp = (ctypes.c_char_p * (len(env_list) + 1))()
        for i, env_str in enumerate(env_list):
            envp[i] = env_str.encode('utf-8')
        envp[len(env_list)] = None
        return envp
    return _make

# TODO: moves neccesary executables scripts there.
# ...existing code...
@pytest.fixture
def test_create_workspace(request):
    """Create a workspace folder under tests/integration/backend/test-workspaces/<name>.

    Returns a factory callable _make(name=None) that creates a workspace directory
    and records it for cleanup. Default name uses the requesting test node name.
    """
    created: list[Path] = []

    def _make(name: str | None = None) -> Path:
        base_name = name if name is not None else request.node.name
        backend_dir = Path(__file__).parent  # tests/integration/backend
        root = backend_dir / "test-workspaces"
        root.mkdir(parents=True, exist_ok=True)

        workspace = root / base_name
        # start fresh for deterministic debugging: remove if already exists
        if workspace.exists():
            shutil.rmtree(workspace)
        workspace.mkdir(parents=True, exist_ok=False)

        created.append(workspace)
        return workspace

    yield _make

    # teardown: remove all created workspaces
    for p in created:
        try:
            if p.exists():
                shutil.rmtree(p)
        except Exception:
            pass


@pytest.fixture
def test_context(test_api_lib):
    """Factory fixture for creating and cleaning up test contexts."""
    contexts = []
    
    def _create(cmds, cmd_count, envp, last_status):
        ctx = test_api_lib.msh_test_ctx_create(cmds, cmd_count, envp, last_status)
        contexts.append(ctx)
        return ctx
    
    yield _create
    
    # Cleanup all contexts
    for ctx in contexts:
        if ctx:
            test_api_lib.msh_test_ctx_destroy(ctx)


@pytest.fixture
def test_runner_tty(test_create_workspace, request):
    """Spawn the test runner executable inside a temporary workspace named for the test.

    The runner binary is copied into the workspace and made executable. While the
    runner is active the current working directory is the workspace; cwd is restored
    on teardown.
    """
    workspace = test_create_workspace(f"{request.node.name}_runner")
    runner_src = Path(__file__).parent.parent / "apis" / "test_runner_exec"

    if not runner_src.exists():
        pytest.fail(f"Test runner not found: {runner_src}")

    runner_dest = workspace / "test_runner_exec"
    shutil.copy(str(runner_src), str(runner_dest))
    runner_dest.chmod(runner_dest.stat().st_mode | 0o111)

    old_cwd = Path.cwd()
    os.chdir(str(workspace))
    try:
        proc = pexpect.spawn(str(runner_dest), encoding='utf-8', timeout=5)
    except Exception:
        os.chdir(str(old_cwd))
        raise

    yield proc

    # cleanup
    try:
        proc.sendline("EXIT")
        proc.expect(pexpect.EOF, timeout=1)
    except Exception:
        pass
    try:
        proc.close()
    except Exception:
        pass
    os.chdir(str(old_cwd))
