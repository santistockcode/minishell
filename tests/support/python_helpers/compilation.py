"""
Shared compilation utilities for test scripts.
"""

import subprocess
from pathlib import Path
from .colors import Colors


def ensure_libft_built(libft_dir: Path) -> bool:
    """Build libft if not already built."""
    libft_lib = libft_dir / "bin" / "libft.a"
    if not libft_lib.exists():
        print(Colors.info("Building libft..."))
        result = subprocess.run(
            ["make", "-C", str(libft_dir)],
            capture_output=True,
            text=True
        )
        if result.returncode != 0:
            print(Colors.error(f"Failed to build libft:\n{result.stderr}"))
            return False
        print(Colors.success("Built libft"))
    return True


def run_command(cmd: list, capture: bool = True) -> subprocess.CompletedProcess:
    """Run a shell command."""
    if capture:
        return subprocess.run(cmd, capture_output=True, text=True)
    else:
        return subprocess.run(cmd)