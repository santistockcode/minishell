"""
ANSI color codes for terminal output.
"""


class Colors:
    """ANSI color codes for pretty terminal output."""
    
    # Basic colors
    BLACK = '\033[0;30m'
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[0;33m'
    BLUE = '\033[0;34m'
    MAGENTA = '\033[0;35m'
    CYAN = '\033[0;36m'
    WHITE = '\033[0;37m'
    
    # Bold colors
    BOLD_BLACK = '\033[1;30m'
    BOLD_RED = '\033[1;31m'
    BOLD_GREEN = '\033[1;32m'
    BOLD_YELLOW = '\033[1;33m'
    BOLD_BLUE = '\033[1;34m'
    BOLD_MAGENTA = '\033[1;35m'
    BOLD_CYAN = '\033[1;36m'
    BOLD_WHITE = '\033[1;37m'
    
    # Backgrounds
    BG_BLACK = '\033[40m'
    BG_RED = '\033[41m'
    BG_GREEN = '\033[42m'
    BG_YELLOW = '\033[43m'
    BG_BLUE = '\033[44m'
    BG_MAGENTA = '\033[45m'
    BG_CYAN = '\033[46m'
    BG_WHITE = '\033[47m'
    
    # Special
    BOLD = '\033[1m'
    DIM = '\033[2m'
    ITALIC = '\033[3m'
    UNDERLINE = '\033[4m'
    BLINK = '\033[5m'
    REVERSE = '\033[7m'
    HIDDEN = '\033[8m'
    
    # Reset
    RESET = '\033[0m'
    RESET_BOLD = '\033[21m'
    RESET_DIM = '\033[22m'
    RESET_UNDERLINE = '\033[24m'
    RESET_BLINK = '\033[25m'
    RESET_REVERSE = '\033[27m'
    RESET_HIDDEN = '\033[28m'
    
    @staticmethod
    def colorize(text: str, color: str) -> str:
        """Wrap text with color codes."""
        return f"{color}{text}{Colors.RESET}"
    
    @staticmethod
    def success(text: str) -> str:
        """Format success message."""
        return Colors.colorize(f"✓ {text}", Colors.BOLD_GREEN)
    
    @staticmethod
    def error(text: str) -> str:
        """Format error message."""
        return Colors.colorize(f"✗ {text}", Colors.BOLD_RED)
    
    @staticmethod
    def warning(text: str) -> str:
        """Format warning message."""
        return Colors.colorize(f"⚠ {text}", Colors.BOLD_YELLOW)
    
    @staticmethod
    def info(text: str) -> str:
        """Format info message."""
        return Colors.colorize(f"ℹ {text}", Colors.BOLD_BLUE)
    
    @staticmethod
    def header(text: str) -> str:
        """Format header."""
        return Colors.colorize(text, Colors.BOLD + Colors.CYAN)