#include "exec_cmds_helpers.h"
#include "test_helpers.h"
#include "../../../Libft/include/libft.h"
#include "../../../include/syswrap.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

/* ============================================================================
 * TEST FILE MANAGEMENT
 * ============================================================================ */

int create_test_input_file(const char *path, const char *content)
{
    int fd;
    
    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
        return (-1);
    if (content && *content)
        write(fd, content, ft_strlen(content));
    close(fd);
    return (0);
}

char *read_test_output_file(const char *path)
{
    int     fd;
    char    *content;
    char    buf[4096];
    ssize_t bytes_read;
    size_t  total;
    
    fd = open(path, O_RDONLY);
    if (fd == -1)
        return (NULL);
    total = 0;
    while ((bytes_read = read(fd, buf + total, sizeof(buf) - total - 1)) > 0)
        total += bytes_read;
    close(fd);
    buf[total] = '\0';
    content = ft_strdup(buf);
    return (content);
}

void cleanup_fd_test_files(void)
{
    unlink(FD_TEST_INPUT);
    unlink(FD_TEST_OUTPUT);
    unlink(FD_TEST_INPUT_2);
    unlink(FD_TEST_OUTPUT_2);
    unlink(FD_TEST_INPUT_3);
    unlink(FD_TEST_OUTPUT_3);
}

int verify_file_content(const char *path, const char *expected)
{
    char *content;
    int result;
    
    content = read_test_output_file(path);
    if (!content)
        return (-1);
    result = (ft_strncmp(content, expected, ft_strlen(expected)) == 0) ? 0 : -1;
    free(content);
    return (result);
}

int verify_file_empty(const char *path)
{
    struct stat st;
    
    if (stat(path, &st) == -1)
        return (-1);
    return (st.st_size == 0) ? 0 : -1;
}

int verify_file_not_exists(const char *path)
{
    return (access(path, F_OK) == -1) ? 0 : -1;
}

/* ============================================================================
 * COMMAND BUILDERS
 * ============================================================================ */

t_cmd *fd_make_cmd(const char **argv, int argc)
{
    return (new_cmd_from_args(argv, argc));
}

t_cmd *fd_make_cmd_redir_in(const char **argv, int argc, const char *in_file)
{
    t_cmd   *cmd;
    t_redir *redir;
    
    cmd = new_cmd_from_args(argv, argc);
    if (!cmd)
        return (NULL);
    redir = make_redir(R_IN, in_file, 0, -1);
    cmd->redirs = ft_lstnew(redir);
    return (cmd);
}

t_cmd *fd_make_cmd_redir_out(const char **argv, int argc, const char *out_file)
{
    t_cmd   *cmd;
    t_redir *redir;
    
    cmd = new_cmd_from_args(argv, argc);
    if (!cmd)
        return (NULL);
    redir = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    cmd->redirs = ft_lstnew(redir);
    return (cmd);
}

t_cmd *fd_make_cmd_redir_in_out(const char **argv, int argc, 
                                 const char *in_file, const char *out_file)
{
    t_cmd   *cmd;
    t_redir *redir_in;
    t_redir *redir_out;
    
    cmd = new_cmd_from_args(argv, argc);
    if (!cmd)
        return (NULL);
    redir_in = make_redir(R_IN, in_file, 0, -1);
    redir_out = make_redir(R_OUT_TRUNC, out_file, 0, -1);
    cmd->redirs = ft_lstnew(redir_in);
    ft_lstadd_back(&cmd->redirs, ft_lstnew(redir_out));
    return (cmd);
}

t_cmd *fd_make_cmd_redir_append(const char **argv, int argc, const char *out_file)
{
    t_cmd   *cmd;
    t_redir *redir;
    
    cmd = new_cmd_from_args(argv, argc);
    if (!cmd)
        return (NULL);
    redir = make_redir(R_OUT_APPEND, out_file, 0, -1);
    cmd->redirs = ft_lstnew(redir);
    return (cmd);
}

/* ============================================================================
 * PIPELINE BUILDERS
 * ============================================================================ */

static t_cmd *build_cmd_with_redir(const char **argv, int argc, 
                                    t_redir_config redir,
                                    const char *in_file, const char *out_file)
{
    if (redir == REDIR_NONE)
        return (fd_make_cmd(argv, argc));
    else if (redir == REDIR_IN)
        return (fd_make_cmd_redir_in(argv, argc, in_file));
    else if (redir == REDIR_OUT)
        return (fd_make_cmd_redir_out(argv, argc, out_file));
    else if (redir == REDIR_IN_OUT)
        return (fd_make_cmd_redir_in_out(argv, argc, in_file, out_file));
    return (NULL);
}

t_list *fd_build_pipeline(t_pipeline_config *config)
{
    t_list  *cmds;
    t_cmd   *cmd1;
    t_cmd   *cmd2;
    t_cmd   *cmd3;
    
    cmds = NULL;
    cmd1 = build_cmd_with_redir(config->argv1, config->argc1, config->redir1,
                                 config->in_file, config->out_file);
    cmd2 = build_cmd_with_redir(config->argv2, config->argc2, config->redir2,
                                 config->in_file, config->out_file);
    cmd3 = build_cmd_with_redir(config->argv3, config->argc3, config->redir3,
                                 config->in_file, config->out_file);
    ft_lstadd_back(&cmds, ft_lstnew(cmd1));
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    return (cmds);
}

t_pipeline_config fd_config_builtin_3_stage(void)
{
    t_pipeline_config config = {
        .argv1 = {"echo", "hello", NULL, NULL},
        .argc1 = 2,
        .redir1 = REDIR_NONE,
        .argv2 = {"echo", "world", NULL, NULL},
        .argc2 = 2,
        .redir2 = REDIR_NONE,
        .argv3 = {"echo", "done", NULL, NULL},
        .argc3 = 2,
        .redir3 = REDIR_NONE,
        .in_file = NULL,
        .out_file = NULL
    };
    return (config);
}

t_pipeline_config fd_config_builtin_3_stage_all_redirs(void)
{
    t_pipeline_config config = {
        .argv1 = {"echo", NULL, NULL, NULL},
        .argc1 = 1,
        .redir1 = REDIR_IN,
        .argv2 = {"echo", NULL, NULL, NULL},
        .argc2 = 1,
        .redir2 = REDIR_NONE,
        .argv3 = {"echo", NULL, NULL, NULL},
        .argc3 = 1,
        .redir3 = REDIR_OUT,
        .in_file = FD_TEST_INPUT,
        .out_file = FD_TEST_OUTPUT
    };
    return (config);
}

t_pipeline_config fd_config_external_3_stage(void)
{
    t_pipeline_config config = {
        .argv1 = {"cat", NULL, NULL, NULL},
        .argc1 = 1,
        .redir1 = REDIR_NONE,
        .argv2 = {"head", "-n", "1", NULL},
        .argc2 = 3,
        .redir2 = REDIR_NONE,
        .argv3 = {"wc", "-c", NULL, NULL},
        .argc3 = 2,
        .redir3 = REDIR_NONE,
        .in_file = NULL,
        .out_file = NULL
    };
    return (config);
}

t_pipeline_config fd_config_external_3_stage_all_redirs(void)
{
    t_pipeline_config config = {
        .argv1 = {"cat", NULL, NULL, NULL},
        .argc1 = 1,
        .redir1 = REDIR_IN,
        .argv2 = {"head", "-n", "1", NULL},
        .argc2 = 3,
        .redir2 = REDIR_NONE,
        .argv3 = {"wc", "-c", NULL, NULL},
        .argc3 = 2,
        .redir3 = REDIR_OUT,
        .in_file = FD_TEST_INPUT,
        .out_file = FD_TEST_OUTPUT
    };
    return (config);
}

/* ============================================================================
 * FD VERIFICATION HELPERS
 * ============================================================================ */

static int is_valgrind_fd(const char *target)
{
    if (!target)
        return (0);
    return (ft_strnstr(target, "valgrind", ft_strlen(target)) != NULL);
}

int count_open_fds(void)
{
    DIR             *dir;
    struct dirent   *entry;
    char            proc_path[64];
    char            link_target[256];
    int             count;
    int             fd_num;
    ssize_t         len;
    
    count = 0;
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/fd", getpid());
    dir = opendir(proc_path);
    if (!dir)
        return (-1);
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        fd_num = atoi(entry->d_name);
        if (fd_num == dirfd(dir))
            continue;
        snprintf(proc_path, sizeof(proc_path), "/proc/%d/fd/%d", getpid(), fd_num);
        len = readlink(proc_path, link_target, sizeof(link_target) - 1);
        if (len > 0)
        {
            link_target[len] = '\0';
            if (!is_valgrind_fd(link_target))
                count++;
        }
    }
    closedir(dir);
    return (count);
}

t_fd_snapshot *take_fd_snapshot(void)
{
    DIR             *dir;
    struct dirent   *entry;
    char            proc_path[64];
    t_fd_snapshot   *snap;
    int             fd_num;
    ssize_t         len;
    
    snap = malloc(sizeof(t_fd_snapshot));
    if (!snap)
        return (NULL);
    snap->count = 0;
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/fd", getpid());
    dir = opendir(proc_path);
    if (!dir)
    {
        free(snap);
        return (NULL);
    }
    while ((entry = readdir(dir)) != NULL && snap->count < 256)
    {
        if (entry->d_name[0] == '.')
            continue;
        fd_num = atoi(entry->d_name);
        if (fd_num == dirfd(dir))
            continue;
        snap->fds[snap->count] = fd_num;
        snprintf(proc_path, sizeof(proc_path), "/proc/%d/fd/%d", getpid(), fd_num);
        len = readlink(proc_path, snap->targets[snap->count], 255);
        if (len > 0)
            snap->targets[snap->count][len] = '\0';
        else
            snap->targets[snap->count][0] = '\0';
        if (!is_valgrind_fd(snap->targets[snap->count]))
            snap->count++;
    }
    closedir(dir);
    return (snap);
}

void free_fd_snapshot(t_fd_snapshot *snap)
{
    free(snap);
}

int compare_fd_snapshots(t_fd_snapshot *before, t_fd_snapshot *after)
{
    if (before->count != after->count)
        return (after->count - before->count); // Positive = leaked fds
    return (0);
}

void print_fd_snapshot(t_fd_snapshot *snap, const char *label)
{
    int i;
    
    fprintf(stderr, "[FD SNAPSHOT: %s] %d fds:\n", label, snap->count);
    for (i = 0; i < snap->count; i++)
    {
        fprintf(stderr, "  fd %d -> %s\n", snap->fds[i], snap->targets[i]);
    }
}

int verify_only_std_fds_open(void)
{
    int count;
    
    count = count_open_fds();
    // Should only have 0, 1, 2 (stdin, stdout, stderr) = 3 fds
    return (count == 3) ? 0 : count - 3;
}

/* ============================================================================
 * MOCK HELPERS
 * ============================================================================ */

static int g_mocks_active = 0;
typedef struct s_mock_state
{
    int call_count;
    int fail_at;
}   t_mock_state;

static t_mock_state g_fork_mock = {0, 0};
static t_mock_state g_open_mock = {0, 0};
static t_mock_state g_dup_mock = {0, 0};
static t_mock_state g_dup2_mock = {0, 0};
static t_mock_state g_pipe_mock = {0, 0};


static int fork_mock_nth(void) {
    errno = EAGAIN;
    return -1;
}

static int open_wrap_eaccess(const char *path, int oflag, int mode) 
{
    if (g_open_mock.call_count == g_open_mock.fail_at)
    {
        errno = EACCES;
        return -1;
    }
    g_open_mock.call_count++;
    return open(path, oflag, mode);
}

static int dup_mock(int fd)
{
    if (g_dup_mock.call_count == g_dup_mock.fail_at)
    {
        errno = EMFILE;
        return -1;
    }
    g_dup_mock.call_count++;
    return dup(fd);
}

static int dup2_mock(int fd, int fd2)
{
    if (g_dup2_mock.call_count == g_dup2_mock.fail_at)
    {
        errno = EBADF;
        return -1;
    }
    g_dup2_mock.call_count++;
    return dup2(fd, fd2);
}

static int pipe_mock_nth(int pipefd[2])
{
    if (g_pipe_mock.call_count == g_pipe_mock.fail_at)
    {
        errno = EMFILE;
        return -1;
    }
    g_pipe_mock.call_count++;
    return pipe(pipefd);
}


void fd_setup_fork_fail_at(int call_num)
{
    g_fork_mock.call_count = 0;
    g_fork_mock.fail_at = call_num;
    syswrap_set_fork((t_fork_fn)fork_mock_nth);
    g_mocks_active |= 1;
}

void fd_setup_open_fail_at(int call_num)
{
    g_open_mock.call_count = 0;
    g_open_mock.fail_at = call_num;
    syswrap_set_open((t_open_fn)open_wrap_eaccess);
    g_mocks_active |= 2;
}

void fd_setup_dup_fail_at(int call_num)
{
    g_dup_mock.call_count = 0;
    g_dup_mock.fail_at = call_num;
    syswrap_set_dup((t_dup_fn)dup_mock);
    g_mocks_active |= 4;
}

void fd_setup_dup2_fail_at(int call_num)
{
    g_dup2_mock.call_count = 0;
    g_dup2_mock.fail_at = call_num;
    syswrap_set_dup2((t_dup2_fn)dup2_mock);
    g_mocks_active |= 8;
}

void fd_setup_pipe_fail_at(int call_num)
{
    g_pipe_mock.call_count = 0;
    g_pipe_mock.fail_at = call_num;
    syswrap_set_pipe((t_pipe_fn)pipe_mock_nth);
    g_mocks_active |= 16;
}

void fd_teardown_all_mocks(void)
{
    if (g_mocks_active & 1)
        syswrap_set_fork(NULL);
    if (g_mocks_active & 2)
        syswrap_set_open(NULL);
    if (g_mocks_active & 4)
        syswrap_set_dup(NULL);
    if (g_mocks_active & 8)
        syswrap_set_dup2(NULL);
    if (g_mocks_active & 16)
        syswrap_set_pipe(NULL);
    g_mocks_active = 0;
}

/* ============================================================================
 * TEST EXECUTION HELPERS
 * ============================================================================ */

int fd_exec_and_wait(t_shell *sh, t_list *cmds)
{
    int status;
    
    status = exec_cmds(sh, cmds);
    // Wait for any remaining children
    while (wait(NULL) > 0)
        ;
    return (status);
}

t_fd_test_result fd_exec_and_verify(t_shell *sh, t_list *cmds, 
                                     const char *expected_output_file)
{
    t_fd_test_result    result;
    t_fd_snapshot       *before;
    t_fd_snapshot       *after;
    
    ft_memset(&result, 0, sizeof(result));
    
    before = take_fd_snapshot();
    
    result.exec_status = exec_cmds(sh, cmds);
    
    // Wait for all children
    result.all_children_reaped = 1;
    while (wait(NULL) > 0)
        ;
    if (errno != ECHILD)
        result.all_children_reaped = 0;
    
    after = take_fd_snapshot();
    
    result.fd_leak_count = compare_fd_snapshots(before, after);
    
    if (expected_output_file)
        result.output_content = read_test_output_file(expected_output_file);
    
    if (result.fd_leak_count != 0)
    {
        print_fd_snapshot(before, "BEFORE exec_cmds");
        print_fd_snapshot(after, "AFTER exec_cmds");
    }
    
    free_fd_snapshot(before);
    free_fd_snapshot(after);
    
    return (result);
}

void free_fd_test_result(t_fd_test_result *result)
{
    if (result->output_content)
        free(result->output_content);
}
