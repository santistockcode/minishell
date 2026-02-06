#include "test_helpers.h"
#include "../../Libft/include/libft.h"
#include "../../include/minishell.h"
#include <stdlib.h>
#include <string.h>

/*
 * Helper to create a linked list of commands for recursive middle tests
 * Returns a list of 3 middle commands (cmd2, cmd3, cmd4) - cmd1 would be first_command
 */
t_list *create_middle_cmd_list(void)
{
    t_list *cmds = NULL;
    
    // cmd2: cat (passes through)
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    
    // cmd3: head -n 1 (gets first line)
    const char *argv3[] = {"head", "-n", "1", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 3);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    
    // cmd4: wc -c (counts characters)
    const char *argv4[] = {"wc", "-c", NULL};
    t_cmd *cmd4 = new_cmd_from_args(argv4, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd4));
    
    return cmds;
}

/*
 * Helper to create middle commands with redirections on specific commands
 */
t_list *create_middle_cmd_list_with_redir_in_on_second(void)
{
    t_list *cmds = NULL;
    
    // cmd2: cat < infile.txt (input redir)
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    t_redir *redir = make_redir(R_IN, "tests/unit/mock-files/infile.txt", 0, -1);
    cmd2->redirs = ft_lstnew(redir);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    
    // cmd3: tr 'a-z' 'A-Z'
    const char *argv3[] = {"tr", "a-z", "A-Z", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 3);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    
    // cmd4: wc -c
    const char *argv4[] = {"wc", "-c", NULL};
    t_cmd *cmd4 = new_cmd_from_args(argv4, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd4));
    
    return cmds;
}

t_list *create_middle_cmd_list_with_redir_out_on_third(void)
{
    t_list *cmds = NULL;
    

    // cmd2: cat
    const char *argv2[] = {"cat", NULL};
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    ft_lstadd_back(&cmds, ft_lstnew(cmd2));
    // cmd3: head -n 1
    const char *argv3[] = {"head", "-n", "1", NULL};
    t_cmd *cmd3 = new_cmd_from_args(argv3, 3);
    t_redir *redir = make_redir(R_OUT_TRUNC, "tests/unit/mock-files/test_recursive_out.txt", 0, -1);
    cmd3->redirs = ft_lstnew(redir);
    ft_lstadd_back(&cmds, ft_lstnew(cmd3));
    
    // cmd4: wc -c
    const char *argv4[] = {"wc", "-c", NULL};
    t_cmd *cmd4 = new_cmd_from_args(argv4, 2);
    ft_lstadd_back(&cmds, ft_lstnew(cmd4));
    
    return cmds;
}

/*
 * Helper to wait for all children and collect statuses
 */
void wait_for_all_children(int *statuses, int count)
{
    for (int i = 0; i < count; i++)
    {
        wait(&statuses[i]);
    }
}

/* duplicate argv strings and allocate argv array on heap */
char **dup_argv(const char *const *argv_in, size_t argc)
{
    char **argv;
    size_t i;
    size_t j;
    
    i = 0;
    j = 0;
    argv = malloc(sizeof(char*) * (argc + 1));
    if (!argv) return NULL;
    while(i < argc)
    {
        argv[i] = ft_strdup(argv_in[i]);
        if (!argv[i]) {
            while(j < i)
                free(argv[j++]);
            return (free(argv), NULL);
        }
        i++;
    }
    argv[argc] = NULL;
    return argv;
}

/* create t_cmd owning a heap argv (use dup_argv) */
t_cmd *new_cmd_from_args(const char *const *argv_in, size_t argc)
{
    t_cmd *cmd = malloc(sizeof(*cmd));
    if (!cmd) return NULL;
    cmd->argv = (char **) dup_argv(argv_in, argc);
    cmd->redirs = NULL;
    cmd->stage_io = NULL;
    return cmd;
}

t_redir *make_redir(t_redir_type type, const char *target, int quoted, int fd)
{
    t_redir *redir = malloc(sizeof(*redir));
    if (!redir) return NULL;
    redir->type = type;
    redir->fd = fd;
    redir->target = target ? ft_strdup(target) : NULL;
    redir->quoted = quoted;
    return redir;
}

/* environment helpers */
t_list *deep_copy_env(const char **envp)
{
    t_list *env_list = NULL;
    for (int i = 0; envp && envp[i]; ++i) {
        char *eq = ft_strchr(envp[i], '=');
        if (!eq) continue;
        t_env *e = malloc(sizeof(*e));
        if (!e) continue;
        e->key = ft_substr(envp[i], 0, eq - envp[i]);
        e->value = ft_strdup(eq + 1);
        ft_lstadd_back(&env_list, ft_lstnew(e));
    }
    return env_list;
}

void free_env_struct(void *env)
{
    if (!env) return;
    free(((t_env*)env)->key);
    free(((t_env*)env)->value);
    free(env);
}

void free_shell(t_shell *sh)
{
    if (!sh) return;
    ft_lstclear(&sh->env, free_env_struct);
    if (sh->last_err_op)
        free(sh->last_err_op);
    free(sh);
}


t_shell *create_test_shell(const char **test_env, int last_status)
{
    t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
    if (!sh)
        return NULL;

    sh->last_status = last_status;
    sh->should_exit = 0;
    sh->env = deep_copy_env(test_env);
    sh->last_err_op = NULL;
    sh->last_errno = 0;
    sh->save_in = -1;
    sh->save_out = -1;
    sh->save_err = -1;
    sh->cmds_start = NULL;
    return sh;
}


/* Create a pipe with test data written to it, return read end */
int create_mock_pipe_with_data(const char *data)
{
    int temp_pipe[2];
    size_t len;
    
    if (pipe(temp_pipe) == -1)
        return -1;
    
    len = data ? ft_strlen(data) : 0;
    if (len > 0)
    {
        if (write(temp_pipe[1], data, len) == -1)
        {
            close(temp_pipe[0]);
            close(temp_pipe[1]);
            return -1;
        }
    }
    close(temp_pipe[1]);
    return temp_pipe[0];
}


int open_temp_file(void)
{
    int fd = open("tests/unit/mock-files/tmp_helper_builtins.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1)
        perror("open");
    return fd;
}

char *read_temp_file(int fd)
{
    lseek(fd, 0, SEEK_SET);
    char buf[1024];
    ssize_t n = read(fd, buf, sizeof(buf)-1);
    if (n < 0) n = 0;
    buf[n] = '\0';
    return strdup(buf);
}