
from asyncio import sleep
import os
import pytest
import sys
from pathlib import Path
import pexpect
import re
ansi = re.compile(r'\x1B\[[0-?]*[ -/]*[@-~]')

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

    # Add redirection: output of wc to file lookout
    test_runner_tty.sendline("ADD_REDIR 2 1 0 lookout 0")
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
    try:
        test_runner_tty.expect(r"RESULT\s+\d+", timeout=15)
    except pexpect.TIMEOUT:
        print(f"TIMEOUT! Buffer: {repr(test_runner_tty.before)}")
        # Try to see what processes are running
        import subprocess
        subprocess.run(["ps", "aux"], capture_output=False)
        raise

    # # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

    output_file = Path("lookout")

    with output_file.open() as f:
        content = f.read()
        assert content == "4\n"

    # output_file.unlink()

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


# '< no-file grep hello | wc > output': Should create output file
def test_no_file_grep_hello_wc_creates_output_file(test_runner_tty):
    test_runner_tty.sendline("CREATE 2 0")
    test_runner_tty.expect("OK")

    # Add first command: grep hello
    test_runner_tty.sendline("ADD_CMD 0 2 grep hello")
    test_runner_tty.expect("OK")

    # Add second command: wc
    test_runner_tty.sendline("ADD_CMD 1 2 wc")
    test_runner_tty.expect("OK")

    # Add redirection: input of grep to file
    test_runner_tty.sendline("ADD_REDIR 0 0 0 invent 0")
    test_runner_tty.expect("OK")

    # Add redirection: output of wc to file output
    test_runner_tty.sendline("ADD_REDIR 1 1 0 ../mocks/output-test 0")
    test_runner_tty.expect("OK")

    # Set environment variables
    test_runner_tty.sendline(
        "CREATE_ENVP PATH=/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    )
    test_runner_tty.expect("OK")

    # Build the context
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    # Execute the pipeline
    test_runner_tty.sendline("EXEC_PIPELINE")

    test_runner_tty.expect(r"minishell: invent: No such file or directory")

    # # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

    output_file = Path("../mocks/output-test")

    with output_file.open() as f:
        content = f.read()
        assert content == "      0       0       0\n"

    output_file.unlink()

    test_runner_tty.sendline("EXIT")


# cat input-example.txt > out1 > out2: if out1 is created successfully, test that out2 is also created (even if out1 is empty)
def test_cat_infile_to_outfile_creates_both_files(test_runner_tty):

    # First create input-example.txt

    input_file = Path("input-example.txt")
    input_file.write_text("Lorem ipsum\n")

    test_runner_tty.sendline("CREATE 2 0")
    test_runner_tty.expect("OK")

    # Add first command: cat < infile
    test_runner_tty.sendline("ADD_CMD 0 2 cat input-example.txt")
    test_runner_tty.expect("OK")

    # Add second command: > out1
    test_runner_tty.sendline("ADD_REDIR 0 1 0 out1 0")
    test_runner_tty.expect("OK")

    # Add third command: > out2
    test_runner_tty.sendline("ADD_REDIR 0 1 0 out2 0")
    test_runner_tty.expect("OK")

    # Set environment variables
    test_runner_tty.sendline(
        "CREATE_ENVP PATH=/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    )
    test_runner_tty.expect("OK")

    # Build the context
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    # Execute the pipeline
    test_runner_tty.sendline("EXEC_SIMPLE")

    test_runner_tty.expect("RESULT 0")

    # # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

    output_file1 = Path("out1")
    output_file2 = Path("out2")

    with output_file1.open() as f:
        content = f.read()
        assert content == ""

    with output_file2.open() as f:
        content = f.read()
        assert content == "Lorem ipsum\n"

    # output_file1.unlink()
    # output_file2.unlink()
    input_file.unlink()

    test_runner_tty.sendline("EXIT")



# cat < nonexistant > out1 > out2 shouldn't create any out file
def test_cat_nonexistent_infile_to_outfile_does_not_create_files(test_runner_tty):
    """
    CREATE 1 0 
    OK
    ADD_CMD 0 1 cat
    OK
    ADD_REDIR 0 0 0 nonexistant 0
    OK
    CREATE_ENVP PATH=/NOT    
    OK
    BUILD_CONTEXT
    OK
    EXEC_SIMPLE

    """
    test_runner_tty.sendline("CREATE 1 0")
    test_runner_tty.expect("OK")

    # Add first command: cat < nonexistant
    test_runner_tty.sendline("ADD_CMD 0 1 cat")
    test_runner_tty.expect("OK")

    # Add first command: < nonexistant
    test_runner_tty.sendline("ADD_REDIR 0 0 0 nonexistant 0")
    test_runner_tty.expect("OK")

    # Add first command: > out1
    test_runner_tty.sendline("ADD_REDIR 0 1 0 out1 0")
    test_runner_tty.expect("OK")

    # Add first command: > out2
    test_runner_tty.sendline("ADD_REDIR 0 1 0 out2 0")
    test_runner_tty.expect("OK")

    # Set environment variables
    test_runner_tty.sendline(
        "CREATE_ENVP PATH=/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    )
    test_runner_tty.expect("OK")

    # Build the context
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    # Execute the pipeline
    test_runner_tty.sendline("EXEC_SIMPLE")

    test_runner_tty.expect(r"minishell: nonexistant: No such file or directory")
    test_runner_tty.expect("RESULT 1")

    # # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

    output_file1 = Path("out1")
    output_file2 = Path("out2")

    assert not output_file1.exists()
    assert not output_file2.exists()

    test_runner_tty.sendline("EXIT")


# echo "hello" | invalid | cat > outfile
def test_echo_hello_invalid_command_cat(test_runner_tty):
    test_runner_tty.sendline("CREATE 3 0")
    test_runner_tty.expect("OK")

    # Add first command: echo "hello"
    test_runner_tty.sendline('ADD_CMD 0 2 echo hello')
    test_runner_tty.expect("OK")

    # Add second command: invalid
    test_runner_tty.sendline("ADD_CMD 1 2 invalid")
    test_runner_tty.expect("OK")

    # Add third command: cat
    test_runner_tty.sendline("ADD_CMD 2 1 cat")
    test_runner_tty.expect("OK")

    # Add redir to third command
    test_runner_tty.sendline("ADD_REDIR 2 1 0 outfile 1")
    test_runner_tty.expect("OK")

    # Set environment variables
    test_runner_tty.sendline(
        "CREATE_ENVP PATH=/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    )
    test_runner_tty.expect("OK")

    # Build the context
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    # Execute the pipeline
    test_runner_tty.sendline("EXEC_PIPELINE")
    test_runner_tty.expect(r"invalid: command not found")
    test_runner_tty.expect("RESULT 0")

    # # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

    output_file = Path("outfile")

    assert output_file.exists()

    output_file.unlink()

    test_runner_tty.sendline("EXIT")


def test_exit_on_child_pipeline_doesnt_exit(test_runner_tty):
    test_runner_tty.sendline("CREATE 3 0")
    test_runner_tty.expect("OK")

    # Add first command: echo "hello"
    test_runner_tty.sendline('ADD_CMD 0 2 echo hello')
    test_runner_tty.expect("OK")

    # Add second command: invalid
    test_runner_tty.sendline("ADD_CMD 1 1 invalid")
    test_runner_tty.expect("OK")

    # Add third command: cat
    test_runner_tty.sendline("ADD_CMD 2 1 cat")
    test_runner_tty.expect("OK")

    # Add redir to third command
    test_runner_tty.sendline("ADD_REDIR 2 1 0 outfile 1")
    test_runner_tty.expect("OK")

    # Set environment variables
    test_runner_tty.sendline(
        "CREATE_ENVP PATH=/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    )
    test_runner_tty.expect("OK")

    # Build the context
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    # Execute the pipeline
    test_runner_tty.sendline("EXEC_PIPELINE")
    test_runner_tty.expect(r"invalid: command not found")
    test_runner_tty.expect("RESULT 0")

    # # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

    output_file = Path("outfile")

    assert output_file.exists()

    output_file.unlink()

    test_runner_tty.sendline("EXIT")

def test_exit_on_parent_simple_exits_and_prints(test_runner_tty):
    test_runner_tty.sendline("CREATE 1 0")
    test_runner_tty.expect("OK")

    # Add first command: echo "hello"
    test_runner_tty.sendline('ADD_CMD 0 1 exit')
    test_runner_tty.expect("OK")

    # Set environment variables
    test_runner_tty.sendline(
        "CREATE_ENVP PATH=/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    )
    test_runner_tty.expect("OK")

    # Build the context
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    # Execute the pipeline
    test_runner_tty.sendline("EXEC_SIMPLE")
    test_runner_tty.expect(r"exit")
    test_runner_tty.expect("RESULT 0")

    # # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

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

# Test for every possible t_stage_io (see prepare_stage_io)

# Test for various consequtive out redirections 

# All manual pipex tests

# Exit tiene que hacer overflow:
    # calling exit 258 inside bash, is really calling bash that exists with 258 that then if I do: echo $?
    # prints 2. 
    # y lo mismo para atrás, si llamo a exit con -1

# Test minishell calls another minishell

# Test minishell calls a executable that stucks in a loop, if control+c, only exits the executable

# Test that if I'm in folderB inside of folderA and I delete folderA,
    # no segfault on minishell
    # if I go cd ../ or cd ../.. I have not sefault

