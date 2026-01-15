#!/usr/bin/env python3
"""
Compilation helper for test API shared libraries.
Called by tox to build C code before running tests.
"""

import subprocess
import sys
from pathlib import Path
import argparse

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from python_helpers.colors import Colors


def log(message: str, color: str = ""):
    """Print colored log message."""
    if color:
        print(f"{color}{message}{Colors.RESET}")
    else:
        print(message)


def ensure_libft_built(libft_dir: Path) -> bool:
    """Build libft if not already built."""
    libft_lib = libft_dir / "bin" / "libft.a"
    if not libft_lib.exists():
        log("Building libft...", Colors.BOLD_YELLOW)
        result = subprocess.run(
            ["make", "-C", str(libft_dir)],
            capture_output=True,
            text=True
        )
        if result.returncode != 0:
            log(Colors.error(f"Failed to build libft:\n{result.stderr}"))
            return False
        log(Colors.success("Built libft"))
    return True


def compile_shared_library(project_root: Path, runner: bool = False, debug: bool = False) -> bool:
    """Compile test API into shared library."""
    log("Compiling test API shared library...", Colors.BOLD_BLUE)
    
    test_api_dir = project_root / "tests" / "integration" / "apis"

    src_dir = project_root / "src"
    include_dir = project_root / "include"
    libft_dir = project_root / "Libft"
    libft_lib = libft_dir / "bin" / "libft.a"
    
    runner_c = test_api_dir / "test_runner_exec.c"
    runner_bin = test_api_dir / "test_runner_exec"
    test_api_c = test_api_dir / "test_api_exec.c"
    output_so = test_api_dir / "libtest_api_exec.so"
    
    dependencies = [
        src_dir / "set_here_docs.c",
        src_dir / "free_cmds.c",
        src_dir / "syswrap.c",
        src_dir / "expand_hd.c",
        src_dir / "expand_hd_utils.c",
        src_dir / "unlink_hds.c",
        src_dir / "exec_errors.c",
        src_dir / "crtl.c",
        src_dir / "signals.c",
        src_dir / "exec_utils.c",
        src_dir / "logger.c"
    ]

    debug_flag = "-DDEBUG" if debug else ""

    cmd = [
        "cc",
        "-Wall", "-Wextra", "-Werror", "-g", "-fPIC",
        f"-I{include_dir}",
        f"-I{libft_dir}/include",
        f"-I{test_api_dir}",
        str(runner_c),
        str(test_api_c),
        *[str(d) for d in dependencies],
        str(libft_lib),
        "-L", str(test_api_dir),
        f"-Wl,-rpath,{test_api_dir}",
        "-lreadline",
        debug_flag,
        "-o", str(runner_bin)
    ]
    cmd_compile_shared_library = [
        "cc",
        "-Wall", "-Wextra", "-Werror", "-g", "-fPIC",
        "-shared",
        f"-I{include_dir}",
        f"-I{libft_dir}/include",
        str(test_api_c),
        *[str(d) for d in dependencies],
        str(libft_lib),
        "-lreadline",
        "-lm",
        debug_flag,
        "-o", str(output_so)
    ]

    if (runner):
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            log(Colors.error(f"Compilation failed for intermediate tty test runner (here_docs):\n{result.stderr}"))
            return False

    if cmd_compile_shared_library:
        result = subprocess.run(cmd_compile_shared_library, capture_output=True, text=True)
        if result.returncode != 0:
            log(Colors.error(f"Compilation for shared library ffi failed:\n{result.stderr}"))
            return False

    log(Colors.success(f"Compiled: {output_so}"))
    return True


def cleanup(project_root: Path):
    """Clean up compiled artifacts."""
    log("Cleaning up compiled artifacts...", Colors.BOLD_YELLOW)
    test_api_dir = project_root / "tests" / "integration" / "apis"
    so_file = test_api_dir / "libtest_api_exec.so"
    runner_path = test_api_dir / "test_runner_exec"

    if so_file.exists():
        so_file.unlink()
        log(Colors.success("Cleaned up"))
    if runner_path.exists():
        runner_path.unlink()
        log(Colors.success("Cleaned up"))
    else:
        log(Colors.info("Nothing to clean"))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--clean', action='store_true', help='Clean compiled artifacts')
    parser.add_argument('--runner', action='store_true', help='Build the test runner middle tty for here docs testing')
    parser.add_argument('--logger', action='store_true', help='Enable logger (it helps for me to write integration tests)')
    args = parser.parse_args()
    
    project_root = Path(__file__).parent.parent.parent.parent
    
    if args.clean:
        cleanup(project_root)
        return 0
    
    libft_dir = project_root / "Libft"
    if not ensure_libft_built(libft_dir):
        return 1

    if not compile_shared_library(project_root, args.runner, args.logger):
        return 1
    
    return 0


if __name__ == "__main__":
    sys.exit(main())