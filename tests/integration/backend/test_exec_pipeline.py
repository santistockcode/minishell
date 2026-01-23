
import os
import pytest
import sys
from pathlib import Path
import pexpect

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent.parent / "support"))

from python_helpers.constants import RedirType

# Test integration of exec_pipeline

# simple pipeline of commands produces desired output
def test_echo_something_and_pipe_to_wc_happy_path(test_runner_tty):
    """
    CREATE 2 0
    ADD_CMD 0 2 echo hello
    ADD_CMD 1 1 wc
    ADD_REDIR 1 1 0 test_ouptut5 0
    CREATE_ENVP PATH=/home/saalarco/Dev/minishell/.env/bin:/home/saalarco/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/g
    BUILD_CONTEXT
    EXEC_PIPELINE
    """
    # Create context with two commands
    test_runner_tty.sendline("CREATE 2 0")
    test_runner_tty.expect("OK")

    # Add first command: echo hello
    test_runner_tty.sendline("ADD_CMD 0 2 echo hello")
    test_runner_tty.expect("OK")

    # Add second command: wc
    test_runner_tty.sendline("ADD_CMD 1 1 wc")
    test_runner_tty.expect("OK")

    # Add redirection: output of wc to file test_output42
    test_runner_tty.sendline("ADD_REDIR 1 1 0 test_output42 0")
    test_runner_tty.expect("OK")

    # Set environment variables
    test_runner_tty.sendline(
        "CREATE_ENVP PATH=/home/saalarco/Dev/minishell/.env/bin:/home/saalarco/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/g"
    )
    test_runner_tty.expect("OK")

    # Build the context
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    # Execute the pipeline
    test_runner_tty.sendline("EXEC_PIPELINE")
    test_runner_tty.expect("RESULT 0")

    # # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

    # output_file = Path("test_output42")
    # output_file.unlink()

    test_runner_tty.sendline("EXIT")


# cmd > output1 > output2 creates both files
@pytest.mark.skip("Not implemented yet")
def test_cmd_concatenated_outputs_redirections_creates_files():
    pass

@pytest.mark.skip("Not implemented yet")
def test_mixed_heredocs_and_redirections_pipeline():
    pass

@pytest.mark.skip("Not implemented yet")
def test_on_injected_failing_syscall_via_test_api_exec():
    pass

@pytest.mark.skip("Not implemented yet")
def test_signals_when_running_pipeline():
    pass

@pytest.mark.skip("Not implemented yet")
def test_with_lsof_no_dangling_fds():
    pass

@pytest.mark.skip("Not implemented yet")
def test_sleep_commands_correctly_works():
    pass

@pytest.mark.skip("Not implemented yet")
def test_against_failing_scripts():
    pass

@pytest.mark.skip("Not implemented yet")
def test_that_interacts_with_child_processes():
    pass

# cat < infile >> out1 > out2: if out1 has permission denied, test that out2 is not created
@pytest.mark.skip("Not implemented yet")
def test_cat_infile_to_outfile_with_permission_denied(test_runner_tty):
    pass


# include here rest of pipex tests

