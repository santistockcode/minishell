"""
Integration tests for heredoc functionality.
"""

import os
import pytest
import sys
from pathlib import Path
import pexpect

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent.parent / "support"))

from python_helpers.constants import RedirType

def test_single_heredoc_with_real_readline(test_runner_tty):
    """Test heredoc with real readline via test runner."""

    # Create context with one command having a heredoc
    test_runner_tty.sendline("CREATE 1 0")
    test_runner_tty.expect("OK")

    # Add command: cat << EOF
    test_runner_tty.sendline("ADD_CMD 0 1 cat")
    test_runner_tty.expect("OK")

    # Add heredoc redirection
    test_runner_tty.sendline("ADD_REDIR 0 3 0 EOF 0")  # type=3(HEREDOC), fd=0, target=EOF, quoted=0
    test_runner_tty.expect("OK")

    # BUILD THE CONTEXT - This was missing!
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    # Now run heredocs - this will prompt for input!
    test_runner_tty.sendline("RUN_HEREDOCS")

    # Expect heredoc prompt
    test_runner_tty.expect("> ")
    test_runner_tty.sendline("Hello World")

    test_runner_tty.expect("> ")
    test_runner_tty.sendline("Another line")

    test_runner_tty.expect("> ")
    test_runner_tty.sendline("EOF")  # Delimiter

    # Check result
    test_runner_tty.expect("RESULT 0")
    
    # Get the heredoc target (should be changed to .here_doc_*)
    test_runner_tty.sendline("GET_REDIR_TARGET 0 0")
    test_runner_tty.expect(r"TARGET (\.here_doc_\d+)")

    target = test_runner_tty.match.group(1)
    assert target.startswith(".here_doc_")
    
    # Verify file exists and contains correct content
    heredoc_file = Path(target)
    assert heredoc_file.exists()
    
    content = heredoc_file.read_text()
    assert "Hello World" in content
    assert "Another line" in content
    
    # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")


def test_single_heredoc_target_changed(test_runner_tty):
    """Test that heredoc changes target from delimiter to temp file."""
    # Create context with one command having a heredoc
    test_runner_tty.sendline("CREATE 1 0")
    test_runner_tty.expect("OK")

    # Add command: cat << EOF
    test_runner_tty.sendline("ADD_CMD 0 1 cat")
    test_runner_tty.expect("OK")

    # Add heredoc redirection
    test_runner_tty.sendline("ADD_REDIR 0 3 0 EOF 0")  # type=3(HEREDOC), fd=0, target=EOF, quoted=0
    test_runner_tty.expect("OK")

    # BUILD THE CONTEXT - This was missing!
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    # Now run heredocs - this will prompt for input!
    test_runner_tty.sendline("RUN_HEREDOCS")

    # Expect heredoc prompt
    test_runner_tty.expect("> ")
    test_runner_tty.sendline("Hello World")

    test_runner_tty.expect("> ")
    test_runner_tty.sendline("Another line")

    test_runner_tty.expect("> ")
    test_runner_tty.sendline("EOF")  # Delimiter

    # Check result
    test_runner_tty.expect("RESULT 0")

    # Get the heredoc target (should be changed to .here_doc_*)
    test_runner_tty.sendline("GET_REDIR_TARGET 0 0")
    test_runner_tty.expect(r"TARGET (\.here_doc_\d+)")

    target = test_runner_tty.match.group(1)
    assert target.startswith(".here_doc_")

    # Verify file exists and contains correct content
    heredoc_file = Path(target)
    assert heredoc_file.exists()

    content = heredoc_file.read_text()
    assert "Hello World" in content
    assert "Another line" in content

    # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")


def test_multiple_heredocs_in_pipeline(test_runner_tty):
    """Test multiple heredocs in a pipeline."""
    # Create context with 2 commands
    test_runner_tty.sendline("CREATE 2 0")
    test_runner_tty.expect("OK")

    # Command 1: cat << EOF1
    test_runner_tty.sendline("ADD_CMD 0 1 cat")
    test_runner_tty.expect("OK")
    test_runner_tty.sendline("ADD_REDIR 0 3 0 EOF1 0")
    test_runner_tty.expect("OK")
    
    # Command 2: grep test << EOF2
    test_runner_tty.sendline("ADD_CMD 1 2 grep test")
    test_runner_tty.expect("OK")
    test_runner_tty.sendline("ADD_REDIR 1 3 0 EOF2 0")
    test_runner_tty.expect("OK")

    # Build context
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")
    
    # Run heredocs - will prompt for BOTH heredocs
    test_runner_tty.sendline("RUN_HEREDOCS")

    # First heredoc (EOF1)
    test_runner_tty.expect("> ")
    test_runner_tty.sendline("line 1")
    test_runner_tty.expect("> ")
    test_runner_tty.sendline("line 2")
    test_runner_tty.expect("> ")
    test_runner_tty.sendline("EOF1")

    # Second heredoc (EOF2)
    test_runner_tty.expect("> ")
    test_runner_tty.sendline("more content")
    test_runner_tty.expect("> ")
    test_runner_tty.sendline("EOF2")

    # Check success
    test_runner_tty.expect("RESULT 0")
    
    # Verify both heredocs have unique filenames
    test_runner_tty.sendline("GET_REDIR_TARGET 0 0")
    test_runner_tty.expect(r"TARGET (\.here_doc_\d+)")
    target1 = test_runner_tty.match.group(1)

    test_runner_tty.sendline("GET_REDIR_TARGET 1 0")
    test_runner_tty.expect(r"TARGET (\.here_doc_\d+)")
    target2 = test_runner_tty.match.group(1)

    assert target1 != target2, "Heredoc files should have unique names"
    assert Path(target1).exists()
    assert Path(target2).exists()
    
    # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")


def test_cmds_no_here_docs_returns_0_via_shared_lib(test_api_lib, make_redir_spec, make_cmd_spec, make_envp, test_context):
    """Test that cmds with no here_docs returns 0"""
    # Redirection infile no heredoc
    simple_redir = make_redir_spec(RedirType.R_IN, "fake/path", 0, False)
    cmd = make_cmd_spec(["cat"], [simple_redir])
    cmds_arr = (type(cmd) * 1)(cmd)
    
    envp = make_envp({"VAR": "value"})
    ctx = test_context(cmds_arr, 1, envp, 0)
    
    result = test_api_lib.msh_test_set_here_docs(ctx)
    assert result == 0


def test_heredoc_empty_content(test_runner_tty):
    """Test heredoc with only delimiter (empty content)."""
    test_runner_tty.sendline("CREATE 1 0")
    test_runner_tty.expect("OK")

    test_runner_tty.sendline("ADD_CMD 0 1 cat")
    test_runner_tty.expect("OK")
    test_runner_tty.sendline("ADD_REDIR 0 3 0 EOF 0")
    test_runner_tty.expect("OK")

    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    test_runner_tty.sendline("RUN_HEREDOCS")
    test_runner_tty.expect("> ")
    test_runner_tty.sendline("EOF")  # Immediate delimiter

    test_runner_tty.expect("RESULT 0")
    
    test_runner_tty.sendline("GET_REDIR_TARGET 0 0")
    test_runner_tty.expect(r"TARGET (\.here_doc_\d+)")
    target = test_runner_tty.match.group(1)
    
    # Verify file is empty
    content = Path(target).read_text()
    assert content == ""
    
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")


def test_heredoc_expanded_on_custom_envp(test_runner_tty):
    """Test heredoc with environment variable expansion."""
    test_runner_tty.sendline("CREATE 1 0")
    test_runner_tty.expect("OK")

    test_runner_tty.sendline("ADD_CMD 0 1 cat")
    test_runner_tty.expect("OK")
    test_runner_tty.sendline("ADD_REDIR 0 3 0 EOF 0")
    test_runner_tty.expect("OK")

    # Add environment variable
    test_runner_tty.sendline("CREATE_ENVP USER=mockJoe")
    test_runner_tty.expect("OK")

    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    test_runner_tty.sendline("RUN_HEREDOCS")
    test_runner_tty.expect("> ")
    test_runner_tty.sendline("Take a look the user name is $USER")
    test_runner_tty.expect("> ")
    test_runner_tty.sendline("EOF")

    test_runner_tty.expect("RESULT 0")
    
    test_runner_tty.sendline("GET_REDIR_TARGET 0 0")
    test_runner_tty.expect(r"TARGET (\.here_doc_\d+)")
    target = test_runner_tty.match.group(1)
    
    # Verify file content with expanded environment variable
    content = Path(target).read_text()
    assert content == "Take a look the user name is mockJoe\n"
    
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")

@pytest.mark.skip(reason="FIXME: signals not working")
def test_signal_in_the_middle_of_fetching_here_docs_interrupts_pipeline(test_runner_tty):
    """Test that sending a signal while fetching here docs interrupts the pipeline."""
    # Create context with one command having a heredoc
    test_runner_tty.sendline("CREATE 1 0")
    test_runner_tty.expect("OK")

    # Add command: cat << EOF
    test_runner_tty.sendline("ADD_CMD 0 1 cat")
    test_runner_tty.expect("OK")

    # Add heredoc redirection
    test_runner_tty.sendline("ADD_REDIR 0 3 0 EOF 0")
    test_runner_tty.expect("OK")

    # BUILD THE CONTEXT
    test_runner_tty.sendline("BUILD_CONTEXT")
    test_runner_tty.expect("OK")

    # Run the heredoc
    test_runner_tty.sendline("RUN_HEREDOCS")
    test_runner_tty.expect("> ")
    test_runner_tty.sendline("line 1")
    test_runner_tty.expect("> ")

    # Send ctrl+c signal to interrupt
    test_runner_tty.sendcontrol('c')
    
    # Expect the signal to be caught and return error code
    test_runner_tty.expect("RESULT -1")

    # Cleanup
    test_runner_tty.sendline("DESTROY")
    test_runner_tty.expect("OK")
