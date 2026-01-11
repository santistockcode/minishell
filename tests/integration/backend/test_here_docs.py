"""
Integration tests for heredoc functionality.
"""

import pytest
import sys
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from python_helpers.constants import RedirType


"""
Ejemplo de test super básico: 

Build t_msh_test_redir_spec[] with a sentinel {type=-1}
Build t_msh_test_cmd_spec[] for a pipeline
ctx = msh_test_ctx_create(cmds, n, envp, last_status)
ret = msh_test_set_here_docs(ctx)
use real mini tty with pexpect
Inspect msh_test_get_redir_target(ctx, cmd_i, redir_i) → check for here_doc_* in target
Destroy ctx
"""


def test_single_heredoc_target_changed(test_api_lib, make_redir_spec, make_cmd_spec, make_envp, test_context):
    """Test that heredoc changes target from delimiter to temp file."""
    # Build redirs: one heredoc with delimiter "EOF"
    heredoc_redir = make_redir_spec(RedirType.R_HEREDOC, "EOF", quoted=0)
    
    # Build command: cat with heredoc
    cmd = make_cmd_spec(["cat"], [heredoc_redir])
    cmds_arr = (type(cmd) * 1)(cmd)
    
    # Create environment
    envp = make_envp({"USER": "testuser", "HOME": "/home/test"})
    
    # Create context
    ctx = test_context(cmds_arr, 1, envp, 0)
    assert ctx is not None
    
    # Call set_here_docs
    result = test_api_lib.msh_test_set_here_docs(ctx)
    assert result == 0, "set_here_docs should succeed"
    
    # Inspect target - should be changed to .here_doc_*
    target = test_api_lib.msh_test_get_redir_target(ctx, 0, 0)
    assert target is not None
    target_str = target.decode('utf-8')
    
    assert target_str.startswith(".here_doc_"), \
        f"Expected target to start with '.here_doc_', got: {target_str}"


@pytest.mark.skip(reason="Test not implemented yet")
def test_multiple_heredocs_in_pipeline(test_api_lib, make_redir_spec, make_cmd_spec, make_envp, test_context):
    """Test multiple heredocs in a pipeline."""
    # Command 1: cat << EOF1
    heredoc1 = make_redir_spec(RedirType.R_HEREDOC, "EOF1", quoted=0)
    cmd1 = make_cmd_spec(["cat"], [heredoc1])
    
    # Command 2: grep test << EOF2
    heredoc2 = make_redir_spec(RedirType.R_HEREDOC, "EOF2", quoted=0)
    cmd2 = make_cmd_spec(["grep", "test"], [heredoc2])
    
    cmds_arr = (type(cmd1) * 2)(cmd1, cmd2)
    envp = make_envp({})
    
    ctx = test_context(cmds_arr, 2, envp, 0)
    result = test_api_lib.msh_test_set_here_docs(ctx)
    
    assert result == 0
    
    # Check both heredocs have unique names
    target1 = test_api_lib.msh_test_get_redir_target(ctx, 0, 0).decode('utf-8')
    target2 = test_api_lib.msh_test_get_redir_target(ctx, 1, 0).decode('utf-8')
    
    assert target1.startswith(".here_doc_")
    assert target2.startswith(".here_doc_")
    assert target1 != target2, "Heredoc files should have unique names"


@pytest.mark.skip(reason="Test not implemented yet")
def test_quoted_heredoc_no_expansion(test_api_lib, make_redir_spec, make_cmd_spec, make_envp, test_context):
    """Test that quoted heredocs don't expand variables."""
    # Heredoc with quoted delimiter
    heredoc = make_redir_spec(RedirType.R_HEREDOC, "EOF", quoted=1)
    cmd = make_cmd_spec(["cat"], [heredoc])
    cmds_arr = (type(cmd) * 1)(cmd)
    
    envp = make_envp({"VAR": "value"})
    ctx = test_context(cmds_arr, 1, envp, 0)
    
    result = test_api_lib.msh_test_set_here_docs(ctx)
    assert result == 0