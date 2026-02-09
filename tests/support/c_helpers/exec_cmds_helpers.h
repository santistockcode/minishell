#ifndef EXEC_CMDS_HELPERS_H
#define EXEC_CMDS_HELPERS_H

#include "../../../include/minishell.h"
#include "../../../include/exec.h"

/* ============================================================================
 * TEST FILE PATHS
 * ============================================================================ */

#define FD_TEST_DIR         "tests/manual/fd-testing-exec-cmds"
#define FD_TEST_INPUT       FD_TEST_DIR "/test_input.txt"
#define FD_TEST_OUTPUT      FD_TEST_DIR "/test_output.txt"
#define FD_TEST_INPUT_2     FD_TEST_DIR "/test_input_2.txt"
#define FD_TEST_OUTPUT_2    FD_TEST_DIR "/test_output_2.txt"
#define FD_TEST_INPUT_3     FD_TEST_DIR "/test_input_3.txt"
#define FD_TEST_OUTPUT_3    FD_TEST_DIR "/test_output_3.txt"
#define FD_TEST_NONEXISTENT FD_TEST_DIR "/nonexistent_file.txt"

/* ============================================================================
 * TEST FILE MANAGEMENT
 * ============================================================================ */

/* Create test input file with content */
int     create_test_input_file(const char *path, const char *content);

/* Read content from test output file, returns malloc'd string or NULL */
char    *read_test_output_file(const char *path);

/* Clean up all test files in FD_TEST_DIR */
void    cleanup_fd_test_files(void);

/* Verify file exists and has expected content */
int     verify_file_content(const char *path, const char *expected);

/* Verify file is empty */
int     verify_file_empty(const char *path);

/* Verify file does not exist */
int     verify_file_not_exists(const char *path);

/* ============================================================================
 * COMMAND BUILDERS (using files instead of pipes)
 * ============================================================================ */

/* Create a simple command with no redirections */
t_cmd   *fd_make_cmd(const char **argv, int argc);

/* Create command with input redirection */
t_cmd   *fd_make_cmd_redir_in(const char **argv, int argc, const char *in_file);

/* Create command with output redirection */
t_cmd   *fd_make_cmd_redir_out(const char **argv, int argc, const char *out_file);

/* Create command with both input and output redirections */
t_cmd   *fd_make_cmd_redir_in_out(const char **argv, int argc, 
                                   const char *in_file, const char *out_file);

/* Create command with append redirection */
t_cmd   *fd_make_cmd_redir_append(const char **argv, int argc, const char *out_file);

/* ============================================================================
 * PIPELINE BUILDERS
 * ============================================================================ */

typedef enum e_redir_config {
    REDIR_NONE_TEST      = 0,
    REDIR_IN_TEST       = 1,
    REDIR_OUT_TEST      = 2,
    REDIR_IN_OUT_TEST   = 3
} t_redir_config;

/* Build a 3-stage pipeline with specified redir configs for each stage
 * redir_files[0] = input file for stage with REDIR_IN or REDIR_IN_OUT
 * redir_files[1] = output file for stage with REDIR_OUT or REDIR_IN_OUT
 */
typedef struct s_pipeline_config {
    const char  *argv1[4];      // First command argv
    int         argc1;
    t_redir_config redir1;
    
    const char  *argv2[4];      // Second command argv
    int         argc2;
    t_redir_config redir2;
    
    const char  *argv3[4];      // Third command argv
    int         argc3;
    t_redir_config redir3;
    
    const char  *in_file;       // Input file (for REDIR_IN/REDIR_IN_OUT)
    const char  *out_file;      // Output file (for REDIR_OUT/REDIR_IN_OUT)
} t_pipeline_config;

/* Build pipeline from config */
t_list  *fd_build_pipeline(t_pipeline_config *config);

/* Pre-built pipeline configs for common test cases */
t_pipeline_config fd_config_builtin_3_stage(void);
t_pipeline_config fd_config_builtin_3_stage_all_redirs(void);
t_pipeline_config fd_config_external_3_stage(void);
t_pipeline_config fd_config_external_3_stage_all_redirs(void);

/* ============================================================================
 * FD VERIFICATION HELPERS
 * ============================================================================ */

/* Count currently open file descriptors (excluding valgrind's) */
int     count_open_fds(void);

/* Snapshot current fd state */
typedef struct s_fd_snapshot {
    int     count;
    int     fds[256];       // List of open fd numbers
    char    targets[256][256]; // What each fd points to
} t_fd_snapshot;

t_fd_snapshot   *take_fd_snapshot(void);
void            free_fd_snapshot(t_fd_snapshot *snap);

/* Compare two snapshots, return 0 if identical (excluding valgrind fds) */
int     compare_fd_snapshots(t_fd_snapshot *before, t_fd_snapshot *after);

/* Print fd snapshot for debugging */
void    print_fd_snapshot(t_fd_snapshot *snap, const char *label);

/* Verify that only std fds (0,1,2) are open (plus valgrind) */
int     verify_only_std_fds_open(void);

/* ============================================================================
 * MOCK HELPERS (wrapper around existing mocks)
 * ============================================================================ */

/* Setup helpers with automatic teardown tracking */
void    fd_setup_fork_fail_at(int call_num);
void    fd_setup_open_fail_at(int call_num);
void    fd_setup_dup_fail_at(int call_num);
void    fd_setup_dup2_fail_at(int call_num);
void    fd_setup_pipe_fail_at(int call_num);

/* Teardown all mocks at once */
void    fd_teardown_all_mocks(void);

/* ============================================================================
 * TEST EXECUTION HELPERS
 * ============================================================================ */

/* Execute exec_cmds and wait for all children, returns last status */
int     fd_exec_and_wait(t_shell *sh, t_list *cmds);

/* Execute exec_cmds with fd verification before/after */
typedef struct s_fd_test_result {
    int     exec_status;        // Return value from exec_cmds
    int     fd_leak_count;      // Number of leaked fds (should be 0)
    int     all_children_reaped; // All wait() calls succeeded
    char    *output_content;    // Content of output file (if applicable)
} t_fd_test_result;

t_fd_test_result    fd_exec_and_verify(t_shell *sh, t_list *cmds, 
                                        const char *expected_output_file);
void                free_fd_test_result(t_fd_test_result *result);

#endif