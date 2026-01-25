#!/usr/bin/env python3
"""
Minunit unit test compiler and runner for minishell.
Refactored from unit_tests_exec_part.py
"""

import subprocess
import sys
import argparse
from pathlib import Path
from typing import List, Optional

# Add support directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from python_helpers.colors import Colors
from python_helpers.compilation import ensure_libft_built, run_command


class UnitTestRunner:
    """Compiles and runs Minunit unit tests."""
    
    def __init__(self, debug: bool = False, valgrind: bool = False, keep_binaries: bool = False):
        self.project_root = Path(__file__).parent.parent.parent.parent
        self.test_dir = self.project_root / "tests" / "unit" / "execution"
        self.bin_dir = self.test_dir / "bin"
        self.debug = debug
        self.valgrind = valgrind
        self.keep_binaries = keep_binaries
        self.compiled_binaries = []
        
        self.bin_dir.mkdir(exist_ok=True)
    
    def log(self, message: str, color: str = ""):
        if color:
            print(f"{color}{message}{Colors.RESET}")
        else:
            print(message)
    
    def discover_test_sources(self) -> List[Path]:
        """Find all test_*.c files."""
        return sorted(self.test_dir.glob("test_*.c"))
    
    def get_dependencies_for_test(self, test_file: Path) -> List[Path]:
        """Map test files to their source dependencies."""
        src_dir = self.project_root / "src"
        
        deps_map = {
            "test_set_here_docs.c": [
                "set_here_docs.c", "expand_hd.c", "expand_hd_utils.c",
                "free_cmds.c", "syswrap.c", "unlink_hds.c", "exec_errors.c",
                "signals.c", "crtl.c", "exec_utils.c", "logger.c",
                "path_utils.c", "exit_utils.c", "fds_utils.c"
            ],
            "test_exec_cmds.c": [
                "exec_cmds.c", "exec_single_cmd.c", "free_cmds.c",
                "syswrap.c", "exec_errors.c", "set_here_docs.c", "unlink_hds.c",
                "expand_hd.c", "expand_hd_utils.c", "signals.c", "crtl.c",
                "exec_utils.c", "logger.c", "tuberiex.c", "exec_stage.c",
                "path_utils.c", "exit_utils.c", "builtins/export.c",
                "envp/env_init.c", "envp/free_env.c", "builtins_orq.c", "fds_utils.c",
                "exec_pipeline.c", "exec_simple.c", "do_first_cmd.c",
                "do_middle_cmds.c", "do_last_cmd.c", "prepare_redirs.c",
                "prepare_stage_io.c", "prepare_stage_io_utils.c", "builtins/unset.c"
            ],
            "test_expand_hd.c": [
                "expand_hd.c", "expand_hd_utils.c", "syswrap.c", "logger.c"
            ],
            "test_free_cmds.c": [
                "free_cmds.c", "logger.c"
            ],
            "test_unlink_hds.c": [
                "unlink_hds.c", "syswrap.c", "free_cmds.c", "logger.c",
                "tuberiex.c", "exec_utils.c", "exec_errors.c", "exec_stage.c",
                "builtins/export.c", "path_utils.c", "exit_utils.c",
                "envp/env_init.c", "envp/free_env.c", "builtins_orq.c", "fds_utils.c",
                "builtins/unset.c"
            ],
            "test_exec_errors.c": [
                "exec_errors.c", "syswrap.c", "logger.c"
            ],
            "test_exec_stage.c": [
                "exec_stage.c", "syswrap.c", "logger.c", 
                "exec_errors.c", "exec_utils.c", "free_cmds.c", "tuberiex.c",
                "builtins/export.c", "path_utils.c", "exit_utils.c",
                "envp/env_init.c", "envp/free_env.c", "builtins_orq.c",
                "fds_utils.c", "exec_pipeline.c", "exec_simple.c", "do_first_cmd.c",
                "do_middle_cmds.c", "do_last_cmd.c", "prepare_redirs.c",
                "prepare_stage_io.c", "prepare_stage_io_utils.c", "builtins/unset.c"
            ],
            "test_exec_simple.c": [
                "exec_stage.c", "syswrap.c", "logger.c", 
                "exec_errors.c", "exec_utils.c", "free_cmds.c", "tuberiex.c",
                "builtins/export.c", "path_utils.c", "exit_utils.c",
                "envp/env_init.c", "envp/free_env.c", "builtins_orq.c",
                "fds_utils.c", "exec_pipeline.c", "exec_simple.c", "do_first_cmd.c",
                "do_middle_cmds.c", "do_last_cmd.c", "prepare_redirs.c",
                "prepare_stage_io.c", "prepare_stage_io_utils.c", "builtins/unset.c"    
            ]
        }
        
        deps = deps_map.get(test_file.name, [])
        return [src_dir / dep for dep in deps]
    
    def compile_test(self, test_file: Path) -> Optional[Path]:
        """Compile a single test file."""
        binary_name = test_file.stem
        binary = self.bin_dir / binary_name
        
        self.log(f"Compiling {test_file.name}...", Colors.BOLD_BLUE)
        
        deps = self.get_dependencies_for_test(test_file)
        deps.append(self.project_root / "tests" / "support" / "c_helpers" / "test_helpers.c")

        include_dir = self.project_root / "include"
        third_party_dir = self.project_root / "tests" / "support" / "third_party"
        libft_dir = self.project_root / "Libft"
        libft_lib = libft_dir / "bin" / "libft.a"
        helpers_dir = self.project_root / "tests" / "support" / "c_helpers"

        cmd = [
            "cc",
            "-Wall", "-Wextra", "-Werror",
        ]
        
        if self.debug:
            cmd.extend(["-g3", "-DDEBUG"])
        
        cmd.extend([
            f"-I{include_dir}",
            f"-I{libft_dir}/include",
            f"-I{third_party_dir}",
            f"-I{helpers_dir}",
            str(test_file),
            *[str(d) for d in deps if d.exists()],
            str(libft_lib),
            "-lreadline",
            "-lncurses",
            "-lm",
            "-o", str(binary)
        ])
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            self.log(Colors.error(f"Compilation failed:\n{result.stderr}"))
            return None
        
        self.log(Colors.success(f"Compiled: {binary}"))
        self.compiled_binaries.append(binary)
        return binary
    
    def run_test(self, binary: Path) -> int:
        """Run a compiled test binary."""
        self.log(f"\n{'='*60}", Colors.BOLD_CYAN)
        self.log(f"Running {binary.name}", Colors.BOLD_CYAN)
        self.log(f"{'='*60}", Colors.BOLD_CYAN)
        
        if self.valgrind:
            cmd = [
                "valgrind",
                "--leak-check=full",
                "--show-leak-kinds=all",
                "--track-origins=yes",
                "--log-file={}/valgrind_{}.log".format(self.bin_dir, binary.name),
                str(binary)
            ]
        else:
            cmd = [str(binary)]
        
        result = subprocess.run(cmd)
        
        if result.returncode == 0:
            self.log(Colors.success(f"✓ {binary.name} passed"))
        else:
            self.log(Colors.error(f"✗ {binary.name} failed"))

        return result.returncode
    
    def cleanup_binaries(self):
        """Remove compiled binaries."""
        if self.keep_binaries:
            self.log(Colors.info("Keeping binaries for inspection"))
            return
        
        for binary in self.compiled_binaries:
            if binary.exists():
                binary.unlink()
        
        if self.bin_dir.exists() and not any(self.bin_dir.iterdir()):
            self.bin_dir.rmdir()

        if self.valgrind:
            self.log(Colors.info("Cleaning up Valgrind logs"))
            for binary in self.compiled_binaries:
                log_file = self.bin_dir / f"valgrind_{binary.name}.log"
                if log_file.exists():
                    log_file.unlink()

        self.log(Colors.success("Cleaned up binaries"))
    
    def run_all_tests(self) -> bool:
        """Discover, compile, and run all tests."""
        test_sources = self.discover_test_sources()
        
        if not test_sources:
            self.log(Colors.warning("No test files found"))
            return False
        
        self.log(f"\nFound {len(test_sources)} test file(s)", Colors.BOLD_GREEN)
        
        # Compile all tests
        failed_compilation = []
        for test_file in test_sources:
            binary = self.compile_test(test_file)
            if not binary:
                failed_compilation.append(test_file)
        
        if failed_compilation:
            self.log(Colors.error(f"\n{len(failed_compilation)} test(s) failed to compile"))
            return False
        
        # Run all tests
        failed_tests = []
        for binary in self.compiled_binaries:
            if self.run_test(binary) != 0:
                failed_tests.append(binary)
        
        # Summary
        self.log(f"\n{'='*60}", Colors.BOLD_CYAN)
        self.log("Test Summary", Colors.BOLD_CYAN)
        self.log(f"{'='*60}", Colors.BOLD_CYAN)
        
        total = len(self.compiled_binaries)
        passed = total - len(failed_tests)
        
        self.log(f"Total:  {total}")
        self.log(Colors.success(f"Passed: {passed}"))
        
        if failed_tests:
            self.log(Colors.error(f"Failed: {len(failed_tests)}"))
            for binary in failed_tests:
                self.log(f"  - {binary.name}", Colors.RED)
            return False
        
        return True

    def print_valgrind_summary(self):
        """Print a summary of Valgrind logs."""
        self.log(f"\n{'='*60}", Colors.BOLD_CYAN)
        self.log("Valgrind Summary", Colors.BOLD_CYAN)
        self.log(f"{'='*60}", Colors.BOLD_CYAN)

        for binary in self.compiled_binaries:
            log_file = self.bin_dir / f"valgrind_{binary.name}.log"
            if log_file.exists():
                self.log(f"Valgrind log for {binary.name}:")
                with log_file.open() as f:
                    self.log(f.read())
            else:
                self.log(f"No Valgrind log found for {binary.name}")

        self.log(f"{'='*60}", Colors.BOLD_CYAN)


def main():
    parser = argparse.ArgumentParser(
        description="Minunit unit test compiler and runner",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument('--debug', action='store_true', help='Compile with debug flags')
    parser.add_argument('--valgrind', action='store_true', help='Run tests under Valgrind')
    parser.add_argument('--keep_bin', action='store_true', help='Keep compiled binaries')
    parser.add_argument('--run', action='store_true', help='Run tests after compilation')
    parser.add_argument('--clean', action='store_true', help='Clean compiled binaries')
    
    args = parser.parse_args()
    
    runner = UnitTestRunner(
        debug=args.debug,
        valgrind=args.valgrind,
        keep_binaries=args.keep_bin
    )
    
    if args.clean:
        runner.cleanup_binaries()
        return 0
    
    # Ensure libft is built
    project_root = Path(__file__).parent.parent.parent.parent
    if not ensure_libft_built(project_root / "Libft"):
        return 1
    
    # Compile (and optionally run) tests
    success = runner.run_all_tests() if args.run else True

    # Print summary of valgrind logs
    if args.valgrind:
        runner.print_valgrind_summary()

    # Cleanup unless keeping binaries
    if not args.keep_bin:
        runner.cleanup_binaries()
    
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())