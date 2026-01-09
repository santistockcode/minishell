#!/usr/bin/env python3
"""
Unit test orchestrator for minishell using Criterion.
- Compiles test binaries from tests/pipeline_tests/*.c
- Links project sources and Libft
- Runs Criterion tests and reports status
"""

import argparse
import subprocess
import sys
from pathlib import Path
from typing import List, Optional


class Colors:
    """ANSI color codes for terminal output."""
    RED = '\033[1;31m'
    GREEN = '\033[1;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[1;34m'
    CYAN = '\033[1;36m'
    RESET = '\033[0m'
    BOLD = '\033[1m'


class TestRunner:
    """Compiles and runs Criterion unit tests for minishell."""

    def __init__(self, keep_binaries: bool = False, debug: bool = False, valgrind: bool = False):
        self.keep_binaries = keep_binaries
        # project_root = repo root (parent of tests/)
        self.project_root = Path(__file__).parent.parent
        self.unit_tests_dir = self.project_root / "tests" / "cmds_tests"
        self.src_dir = self.project_root / "src"
        self.include_dir = self.project_root / "include"
        self.libft_dir = self.project_root / "Libft"
        self.libft_lib = self.libft_dir / "bin" / "libft.a"
        self.keep_binaries_valgrind = valgrind

        # Track compiled binaries for cleanup
        self.compiled_binaries: List[Path] = []

        # Compiler settings
        self.cc = "cc"
        base_cflags = ["-Wall", "-Wextra", "-Werror", "-g"]
        if debug:
            base_cflags += ["-g3", "-O0", "-DDEBUG"]
        self.cflags = base_cflags
        self.includes = [
            f"-I{self.include_dir}",
            f"-I{self.libft_dir}/include",
        ]
        # Link miunit and common test helpers
        self.libs = [str(self.libft_lib), "-lm"]
        self.includes.append(f"-I{self.project_root}/tests/third_party")
        self.includes.append(f"-I{self.project_root}/tests/common")

        # Link readline
        self.libs.append("-lreadline")

    def log(self, message: str, color: str = ""):
        """Print a log message."""
        if color:
            print(f"{color}{message}{Colors.RESET}")
        else:
            print(message)
    
    def ensure_libft_built(self) -> bool:
        """Ensure libft is built."""
        if not self.libft_lib.exists():
            self.log("Building libft...", Colors.BLUE)
            try:
                subprocess.run(
                    ["make", "-C", str(self.libft_dir)],
                    capture_output=True,
                    check=True
                )
                return True
            except subprocess.CalledProcessError as e:
                err = e.stderr.decode() if e.stderr else str(e)
                self.log(f"Failed to build libft: {err}", Colors.RED)
                return False
        return True
    
    def discover_test_sources(self) -> List[Path]:
        """Discover all test source files."""
        if not self.unit_tests_dir.exists():
            self.log(f"Tests directory not found: {self.unit_tests_dir}", Colors.RED)
            return []
        test_files = list(self.unit_tests_dir.glob("test_*.c"))
        return test_files
    
    def get_dependencies_for_test(self, test_file: Path) -> List[Path]:
        """Get project source dependencies to link into the test binary."""
        deps: List[Path] = []
        # exec_pipeline tests need the backend impl
        deps.append(self.src_dir / "exec_cmds.c")
        deps.append(self.src_dir / "set_here_docs.c")
        deps.append(self.src_dir / "free_cmds.c")
        deps.append(self.src_dir / "syswrap.c")
        deps.append(self.src_dir / "expand_hd.c")
        deps.append(self.src_dir / "expand_hd_utils.c")
        deps.append(self.src_dir / "unlink_hds.c")
        deps.append(self.src_dir / "exec_errors.c")

        # common test helpers
        deps.append(self.project_root / "tests" / "common" / "test_helpers.c")

        # Add other sources here if future tests require them
        return deps
    
    def compile_test(self, test_file: Path) -> Optional[Path]:
        """Compile a test file."""
        test_name = test_file.stem
        output_binary = self.unit_tests_dir / test_name

        self.log(f"Compiling {test_name}...", Colors.BLUE)

        dependencies = self.get_dependencies_for_test(test_file)

        compile_cmd = [
            self.cc,
            *self.cflags,
            *self.includes,
            str(test_file),
            *[str(dep) for dep in dependencies],
            *self.libs,
            "-o", str(output_binary)
        ]

        try:
            res = subprocess.run(
                compile_cmd,
                capture_output=True,
                check=True
            )
            self.log(f"✓ Compiled {test_name}", Colors.GREEN)
            self.compiled_binaries.append(output_binary)
            return output_binary
        except subprocess.CalledProcessError as e:
            self.log(f"✗ Failed to compile {test_name}", Colors.RED)
            out = e.stdout.decode() if e.stdout else ""
            err = e.stderr.decode() if e.stderr else ""
            if out:
                self.log(out, Colors.RED)
            if err:
                self.log(err, Colors.RED)
            return None
    
    def run_test(self, binary: Path) -> int:
        """Run a test binary - let Criterion handle everything."""
        self.log(f"\nRunning {binary.name}...", Colors.YELLOW)
        if (self.keep_binaries_valgrind):
            self.log(f"Running {binary.name} under Valgrind...", Colors.CYAN)
            result = subprocess.run(["valgrind", "--leak-check=full", "--show-leak-kinds=all", "--track-origins=yes", str(binary)])
            return result.returncode
        result = subprocess.run([str(binary)])
        return result.returncode
    
    def cleanup_binaries(self):
        """Clean up compiled binaries."""
        if self.keep_binaries:
            return
        
        for binary in self.compiled_binaries:
            if binary.exists():
                try:
                    binary.unlink()
                except Exception:
                    pass
    
    def run_unit_tests(self) -> bool:
        """Run unit tests."""
        self.log("\n" + "="*70, Colors.BOLD)
        self.log("UNIT TESTS", Colors.BOLD)
        self.log("="*70, Colors.BOLD)
        if not self.ensure_libft_built():
            return False
        test_files = self.discover_test_sources()
        if not test_files:
            self.log("No test files found", Colors.YELLOW)
            return True
        
        success = True
        
        try:
            for test_file in test_files:
                binary = self.compile_test(test_file)
                if binary and binary.exists():
                    exit_code = self.run_test(binary)
                    if exit_code != 0:
                        success = False
                else:
                    success = False
        finally:
            self.cleanup_binaries()
        
        return success


def cleanup_binaries(self):
    """Clean up compiled binaries in previous executions."""
    if self.keep_binaries:
        return

    for binary in self.compiled_binaries:
        if binary.exists():
            try:
                binary.unlink()
            except Exception:
                pass

def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Criterion unit test runner for minishell",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
    # Run unit tests
    python3 tests/unit_cmds.py
    # Keep compiled binaries for manual execution
    python3 tests/unit_cmds.py --keep-binaries
    # Debug build (adds -DDEBUG -g3 -O0)
    python3 tests/unit_cmds.py --debug
    # Clean binaries
    python3 tests/unit_cmds.py --clean
    # Keep binaries and run under Valgrind
    python3 tests/unit_cmds.py --keep-binaries-valgrind
        """
    )

    # The --help argument is automatically added by argparse, so no need to define it manually.
    
    parser.add_argument(
        '--debug',
        action='store_true',
        help='Compile tests with debug flags'
    )
    
    parser.add_argument(
        '--keep-binaries',
        action='store_true',
        help='Keep compiled test binaries (for manual testing and debugging)'
    )

    parser.add_argument(
        '--clean',
        action='store_true',
        help='Clean up compiled test binaries'
    )

    parser.add_argument(
        '--keep-binaries-valgrind',
        action='store_true',
        help='Keep compiled test binaries and run them under Valgrind'
    )

    args = parser.parse_args()
    
    # If args is none go on with unit tests
    if args is None:
        args = argparse.Namespace()
        args.unit = True
        args.keep_binaries = False
        args.debug = False

    runner = TestRunner(keep_binaries=args.keep_binaries, debug=args.debug, valgrind=args.keep_binaries_valgrind)

    success = True

    if args.clean:
        runner.cleanup_binaries()

    if not runner.run_unit_tests():
        success = False
    
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())