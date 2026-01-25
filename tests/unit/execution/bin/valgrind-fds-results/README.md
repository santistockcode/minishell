# Valgrind File Descriptor Leak Testing

This script runs Valgrind memory and file descriptor leak checks on all executable files in the `tests/unit/execution/bin` directory.

## Usage

```bash
./valgrind-unit-fds-testing.sh
```

## What it does

1. **Validates target directory** - Checks if `./tests/unit/execution/bin` exists
2. **Runs Valgrind on each file** - Executes Valgrind with the following flags:
    - `--leak-check=full` - Detailed memory leak information
    - `--show-leak-kinds=all` - Shows all types of leaks
    - `--trace-children=yes` - Tracks child processes
    - `--child-silent-after-fork=no` - Shows output from child processes
    - `--track-fds=yes` - Tracks file descriptor usage
    - `--track-fds=all` - Shows all file descriptor information
3. **Saves results** - Logs are saved to `valgrind-fds-results/valgrind_<filename>.log`
4. **Displays summary** - Prints all lines containing "FILE" from the results

## Output

Log files are generated in this directory (`valgrind-fds-results/`) with the naming pattern:
```
valgrind_<executable_name>.log
```

The script automatically displays file descriptor-related issues at the end.

## Requirements

- Valgrind must be installed
- Executables must be present in `./tests/unit/execution/bin`