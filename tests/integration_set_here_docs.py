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

Siguientes pasos:
Como exec_cmds y set_here_doc tienen el mismo prototipo no me costará llevarme la implementación a exec_cmds
"""

import sys
import ctypes
import subprocess
import tempfile
import os
from pathlib import Path
from typing import List, Optional
import argparse

#!/usr/bin/env python3
"""
Integration tests for set_here_docs using ctypes and the test_api_set_here_docs wrapper.
Tests real system calls, file creation, and error handling.
"""



class Colors:
    """ANSI color codes for terminal output."""
    RED = '\033[1;31m'
    GREEN = '\033[1;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[1;34m'
    CYAN = '\033[1;36m'
    RESET = '\033[0m'
    BOLD = '\033[1m'


class RedirType:
    """Mirror of t_redir_type from C."""
    R_IN = 0
    R_OUT = 1
    R_APPEND = 2
    R_HEREDOC = 3


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


class IntegrationTestRunner:
    """Compiles and runs integration tests for set_here_docs."""
    
    def __init__(self, keep_binaries: bool = False):
        self.keep_binaries = keep_binaries
        self.project_root = Path(__file__).parent.parent
        self.test_api_dir = self.project_root / "tests" / "integration_test_apis"
        self.src_dir = self.project_root / "src"
        self.include_dir = self.project_root / "include"
        self.libft_dir = self.project_root / "Libft"
        self.libft_lib = self.libft_dir / "bin" / "libft.a"
        
        self.lib_path: Optional[Path] = None
        self.lib = None
        
        # Compiler settings
        self.cc = "cc"
        self.cflags = ["-Wall", "-Wextra", "-Werror", "-g", "-fPIC"]
        self.includes = [
            f"-I{self.include_dir}",
            f"-I{self.libft_dir}/include",
        ]
        self.libs = [str(self.libft_lib), "-lreadline", "-lm"]
    
    def log(self, message: str, color: str = ""):
        """Print a log message."""
        if color:
            print(f"{color}{message}{Colors.RESET}")
        else:
            print(message)
    
    def ensure_libft_built(self) -> bool:
        """Ensure libft is built."""
        if not self.libft_lib.exists():
            self.log("Libft not found. Building...", Colors.YELLOW)
            result = subprocess.run(["make", "-C", str(self.libft_dir)], 
                                   capture_output=True, text=True)
            if result.returncode != 0:
                self.log(f"Failed to build libft: {result.stderr}", Colors.RED)
                return False
        return True
    
    def compile_shared_library(self) -> Optional[Path]:
        """Compile test_api_set_here_docs.c into a shared library."""
        self.log("Compiling test API shared library...", Colors.BLUE)
        
        test_api_c = self.test_api_dir / "test_api_set_here_docs.c"
        output_so = self.test_api_dir / "libtest_api_set_here_docs.so"
        
        dependencies = [
            self.src_dir / "set_here_docs.c",
            self.src_dir / "free_cmds.c",
            self.src_dir / "syswrap.c",
            self.src_dir / "expand_hd.c",
            self.src_dir / "expand_hd_utils.c",
            self.src_dir / "unlink_hds.c",
            self.src_dir / "exec_errors.c",
        ]
        
        cmd = [
            self.cc,
            *self.cflags,
            "-shared",
            *self.includes,
            str(test_api_c),
            *[str(d) for d in dependencies],
            *self.libs,
            "-o", str(output_so)
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            self.log(f"Compilation failed: {result.stderr}", Colors.RED)
            return None
        
        self.log(f"Compiled: {output_so}", Colors.GREEN)
        return output_so
    
    def load_library(self) -> bool:
        """Load the compiled shared library."""
        if not self.lib_path or not self.lib_path.exists():
            self.log("Library not compiled", Colors.RED)
            return False
        
        try:
            self.lib = ctypes.CDLL(str(self.lib_path))
            
            # Define function signatures
            self.lib.msh_test_ctx_create.argtypes = [
                ctypes.POINTER(TestCmdSpec),
                ctypes.c_int,
                ctypes.POINTER(ctypes.c_char_p),
                ctypes.c_int
            ]
            self.lib.msh_test_ctx_create.restype = ctypes.c_void_p
            
            self.lib.msh_test_set_here_docs.argtypes = [ctypes.c_void_p]
            self.lib.msh_test_set_here_docs.restype = ctypes.c_int
            
            self.lib.msh_test_get_redir_target.argtypes = [
                ctypes.c_void_p, ctypes.c_int, ctypes.c_int
            ]
            self.lib.msh_test_get_redir_target.restype = ctypes.c_char_p
            
            self.lib.msh_test_get_last_err_op.argtypes = [ctypes.c_void_p]
            self.lib.msh_test_get_last_err_op.restype = ctypes.c_char_p
            
            self.lib.msh_test_get_last_status.argtypes = [ctypes.c_void_p]
            self.lib.msh_test_get_last_status.restype = ctypes.c_int
            
            self.lib.msh_test_ctx_destroy.argtypes = [ctypes.c_void_p]
            self.lib.msh_test_ctx_destroy.restype = None
            
            return True
        except Exception as e:
            self.log(f"Failed to load library: {e}", Colors.RED)
            return False
    
    def cleanup(self):
        """Clean up compiled artifacts."""
        if not self.keep_binaries and self.lib_path and self.lib_path.exists():
            self.log("Cleaning up compiled library...", Colors.YELLOW)
            self.lib_path.unlink()
    
    def run_tests(self) -> bool:
        """Run all integration tests."""
        if not self.ensure_libft_built():
            return False
        
        self.lib_path = self.compile_shared_library()
        if not self.lib_path:
            return False
        
        if not self.load_library():
            return False
        
        test_methods = [
            self.test_single_heredoc_target_changed
        ]
        
        passed = 0
        failed = 0
        
        for test_method in test_methods:
            test_name = test_method.__name__
            try:
                self.log(f"\nRunning: {test_name}", Colors.CYAN)
                test_method()
                self.log(f"✓ PASSED: {test_name}", Colors.GREEN)
                passed += 1
            except AssertionError as e:
                self.log(f"✗ FAILED: {test_name}\n  {e}", Colors.RED)
                failed += 1
            except Exception as e:
                self.log(f"✗ ERROR: {test_name}\n  {e}", Colors.RED)
                failed += 1
        
        self.log(f"\n{'='*60}", Colors.BOLD)
        self.log(f"Results: {passed} passed, {failed} failed", 
                Colors.GREEN if failed == 0 else Colors.RED)
        
        return failed == 0
    
    # ============================================================================
    # Helper methods for building test structures
    # ============================================================================
    
    def make_redir_spec(self, redir_type: int, target: str, 
                       quoted: int = 0, fd: int = 0) -> TestRedirSpec:
        """Create a TestRedirSpec."""
        return TestRedirSpec(
            type=redir_type,
            fd=fd,
            target=target.encode('utf-8'),
            quoted=quoted
        )
    
    def make_sentinel_redir(self) -> TestRedirSpec:
        """Create a sentinel redir (type=-1)."""
        return TestRedirSpec(type=-1, fd=0, target=None, quoted=0)
    
    def make_cmd_spec(self, argv: List[str], 
                     redirs: List[TestRedirSpec]) -> TestCmdSpec:
        """Create a TestCmdSpec."""
        # Create NULL-terminated argv
        argv_arr = (ctypes.c_char_p * (len(argv) + 1))()
        for i, arg in enumerate(argv):
            argv_arr[i] = arg.encode('utf-8')
        argv_arr[len(argv)] = None
        
        # Create sentinel-terminated redirs
        redirs_arr = (TestRedirSpec * (len(redirs) + 1))()
        for i, redir in enumerate(redirs):
            redirs_arr[i] = redir
        redirs_arr[len(redirs)] = self.make_sentinel_redir()
        
        return TestCmdSpec(argv=argv_arr, redirs=redirs_arr)
    
    def make_envp(self, env_dict: dict) -> ctypes.Array:
        """Create a NULL-terminated environment array."""
        env_list = [f"{k}={v}" for k, v in env_dict.items()]
        envp = (ctypes.c_char_p * (len(env_list) + 1))()
        for i, env_str in enumerate(env_list):
            envp[i] = env_str.encode('utf-8')
        envp[len(env_list)] = None
        return envp
    
    # ============================================================================
    # Test cases
    # ============================================================================
    

    # FIXME: Implement. I very much prefer pytest for this, that means tox for requirements (pytest) and another folder structure
    def test_single_heredoc_target_changed(self):
        """
        Build t_msh_test_redir_spec[] with a sentinel {type=-1}
        Build t_msh_test_cmd_spec[] for a pipeline
        ctx = msh_test_ctx_create(cmds, n, envp, last_status)
        ret = msh_test_set_here_docs(ctx)
        Inspect msh_test_get_redir_target(ctx, cmd_i, redir_i) → check for here_doc_* in target
        Destroy ctx
        """
        pass

def main():
    """Main entry point."""
    
    parser = argparse.ArgumentParser(
        description="Integration test runner for set_here_docs",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    
    parser.add_argument(
        '--keep-binaries',
        action='store_true',
        help='Keep compiled shared library after tests'
    )
    
    args = parser.parse_args()
    
    runner = IntegrationTestRunner(keep_binaries=args.keep_binaries)
    
    try:
        success = runner.run_tests()
        return 0 if success else 1
    finally:
        runner.cleanup()


if __name__ == "__main__":
    sys.exit(main())




