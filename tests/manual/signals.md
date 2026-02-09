# ============================================
# TEST 1: Ctrl+C during a running command
# Expected: command dies, shell prints newline, $? = 130
# ============================================
sleep 10
# Press Ctrl+C
echo $?
# Should print: 130

# ============================================
# TEST 2: Ctrl+\ during a running command
# Expected: command dies, shell prints "Quit (core dumped)", $? = 131
# ============================================
sleep 10
# Press Ctrl+\
echo $?
# Should print: 131

# ============================================
# TEST 3: Ctrl+C at empty prompt
# Expected: new prompt line, $? = 130
# ============================================
# Just press Ctrl+C at the prompt
echo $?
# Should print: 130

# ============================================
# TEST 4: Ctrl+\ at empty prompt
# Expected: nothing happens
# ============================================
# Just press Ctrl+\ at the prompt
# Nothing should print, prompt stays

# ============================================
# TEST 5: Ctrl+D at empty prompt
# Expected: shell exits (prints "exit")
# ============================================
# Press Ctrl+D on empty line
# Shell should exit

# ============================================
# TEST 6: Ctrl+D with text on prompt
# Expected: nothing happens (Ctrl+D only exits on empty line)
# ============================================
# Type "hello" then press Ctrl+D (don't press Enter)
# Nothing should happen, cursor stays

# ============================================
# TEST 7: Ctrl+C during heredoc
# Expected: heredoc aborted, back to prompt, $? = 130
# ============================================
cat << EOF
hello
# Press Ctrl+C here
echo $?
# Should print: 130

# ============================================
# TEST 8: Ctrl+D during heredoc (before delimiter)
# Expected: warning printed, heredoc accepted as-is
# ============================================
cat << EOF
hello
# Press Ctrl+D here
# Should print: bash: warning: here-document delimited by end-of-file (wanted `EOF')
# Then "hello" is printed by cat

# ============================================
# TEST 9: Ctrl+C in pipeline
# Expected: all commands die, $? = 130
# ============================================
sleep 10 | sleep 10 | sleep 10
# Press Ctrl+C
echo $?
# Should print: 130

# ============================================
# TEST 10: Ctrl+\ in pipeline
# Expected: all commands die, "Quit (core dumped)", $? = 131
# ============================================
cat | cat | cat
# Press Ctrl+\
echo $?
# Should print: 131

# ============================================
# TEST 11: Ctrl+C doesn't kill shell after pipeline
# Expected: shell still running after Ctrl+C kills pipeline
# ============================================
cat | grep hello
# Press Ctrl+C
echo "shell is alive"
# Should print: shell is alive

# ============================================
# TEST 12: Multiple heredocs with Ctrl+C on second
# Expected: first heredoc collected, second aborted, $? = 130
# ============================================
cat << A << B
first
A
# Press Ctrl+C here (during second heredoc)
echo $?
# Should print: 130