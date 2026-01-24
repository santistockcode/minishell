
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
    ADD_REDIR 1 1 0 test_ouptut42 0
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

    output_file = Path("test_output42")
    output_file.unlink()

    test_runner_tty.sendline("EXIT")


def test_cat_soemething_and_assert_final_file_correct(test_runner_tty):
    # cat anxiety | grep that > outfile42 (result should be 'that')
    test_runner_tty.sendline("CREATE 2 0")
    test_runner_tty.expect("OK")

    # Add first command: cat anxiety
    test_runner_tty.sendline("ADD_CMD 0 2 cat input-example")
    test_runner_tty.expect("OK")

    # Add second command: grep that
    test_runner_tty.sendline("ADD_CMD 1 2 grep psu")
    test_runner_tty.expect("OK")

    # Add redirection: output of grep to file test_output42
    test_runner_tty.sendline("ADD_REDIR 1 1 0 test_output42 0")
    test_runner_tty.expect("OK")

    # Set environment variables
    test_runner_tty.sendline(
        "CREATE_ENVP PATH=/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    )
    test_runner_tty.expect("OK")

    # Build the context
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    anxiety_file = Path("../mocks/input-example")
    destination_file = Path("input-example")
    if anxiety_file.exists():
        destination_file.write_text(anxiety_file.read_text())
    else:
        raise FileNotFoundError(f"{anxiety_file} does not exist")

    # Execute the pipeline
    test_runner_tty.sendline("EXEC_PIPELINE")
    test_runner_tty.expect("RESULT 0")

    # # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

    output_file = Path("test_output42")

    with output_file.open() as f:
        content = f.read()
        assert content == "ipsum dolor sit amet\n"

    output_file.unlink()

    test_runner_tty.sendline("EXIT")


def test_four_commands_pipeline_and_assert_final_outfile_correct(test_runner_tty):
    # cat input-example | grep psu | wc -w > output
    test_runner_tty.sendline("CREATE 3 0")
    test_runner_tty.expect("OK")

    # Add first command: cat input-example
    test_runner_tty.sendline("ADD_CMD 0 2 cat input-example")
    test_runner_tty.expect("OK")

    # Add second command: grep psu
    test_runner_tty.sendline("ADD_CMD 1 2 grep psu")
    test_runner_tty.expect("OK")

    # Add third command: wc -w
    test_runner_tty.sendline("ADD_CMD 2 2 wc -w")
    test_runner_tty.expect("OK")

    # Add redirection: output of wc to file output
    test_runner_tty.sendline("ADD_REDIR 2 1 0 output 0")
    test_runner_tty.expect("OK")

    # Set environment variables
    test_runner_tty.sendline(
        "CREATE_ENVP PATH=/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    )
    test_runner_tty.expect("OK")

    # Build the context
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    anxiety_file = Path("../mocks/input-example")
    destination_file = Path("input-example")
    if anxiety_file.exists():
        destination_file.write_text(anxiety_file.read_text())
    else:
        raise FileNotFoundError(f"{anxiety_file} does not exist")

    # Execute the pipeline
    test_runner_tty.sendline("EXEC_PIPELINE")
    test_runner_tty.expect("RESULT 0")

    # # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

    output_file = Path("output")

    with output_file.open() as f:
        content = f.read()
        assert content == "4\n"

    output_file.unlink()

    test_runner_tty.sendline("EXIT")


def test_shell_script_correctly_executes(test_runner_tty):
    # ./example.sh | wc -w > outfile
    test_runner_tty.sendline("CREATE 2 0")
    test_runner_tty.expect("OK")

    # Add first command: ./example.sh
    test_runner_tty.sendline("ADD_CMD 0 2 example.sh")
    test_runner_tty.expect("OK")

    # Add second command: wc -w
    test_runner_tty.sendline("ADD_CMD 1 2 wc -w")
    test_runner_tty.expect("OK")

    # Add redirection: output of wc to file outfile
    test_runner_tty.sendline("ADD_REDIR 1 1 0 outfile 0")
    test_runner_tty.expect("OK")

    # Set environment variables
    test_runner_tty.sendline(
        "CREATE_ENVP PATH=/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    )
    test_runner_tty.expect("OK")

    # Build the context
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    # cp example.sh from mocks into folder
    example_file = Path("../mocks/example.sh")
    destination_file = Path("example.sh")
    if example_file.exists():
        destination_file.write_text(example_file.read_text())
    else:
        raise FileNotFoundError(f"{example_file} does not exist")

    # give example.sh execute permissions
    destination_file.chmod(0o755)

    # Execute the pipeline
    test_runner_tty.sendline("EXEC_PIPELINE")
    test_runner_tty.expect("RESULT 0")

    # # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

    output_file = Path("outfile")

    with output_file.open() as f:
        content = f.read()
        assert content == "2\n"

    output_file.unlink()

    test_runner_tty.sendline("EXIT")


# def test_basic_builtin_export_works_as_expected_in_pipe(test_runner_tty):
#     # echo "exporting EXAMPLE_VAR" | export EXAMPLE_VAR=VALUE
#     test_runner_tty.sendline("CREATE 2 0")
#     test_runner_tty.expect("OK")

#     # Add first command: export VAR=VALUE
#     test_runner_tty.sendline("ADD_CMD 0 2 echo 'exporting EXAMPLE_VAR'")
#     test_runner_tty.expect("OK")

#     # Add second command: grep VAR
#     test_runner_tty.sendline("ADD_CMD 1 2 export EXAMPLE_VAR=myvar")
#     test_runner_tty.expect("OK")

#     # Set environment variables
#     test_runner_tty.sendline(
#         "CREATE_ENVP PATH=/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
#     )
#     test_runner_tty.expect("OK")

#     # Build the context
#     test_runner_tty.sendline("BUILD_CONTEXT")
#     test_runner_tty.expect("OK")

#     # Execute the pipeline
#     test_runner_tty.sendline("EXEC_PIPELINE")
#     test_runner_tty.expect("RESULT 0")

#     # Get env variable EXAMPLE_VAR
#     test_runner_tty.sendline("GET_ENV EXAMPLE_VAR")
#     test_runner_tty.expect("VALUE myvar")

#     # # Cleanup
#     test_runner_tty.sendline("DESTROY")
#     test_runner_tty.expect("OK")

#     test_runner_tty.sendline("EXIT")


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

