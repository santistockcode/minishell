/* 
 * Thin executable wrapper for heredoc integration tests.
 * Provides TTY interface for pexpect while using the test API.
 */

#include "test_api_set_here_docs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Protocol:
 * Python sends commands via stdin, this program responds via stdout
 * Commands:
 *   CREATE <cmd_count> <last_status>
 *   ADD_CMD <cmd_index> <argc> <arg1> <arg2> ...
 *   ADD_REDIR <cmd_index> <type> <fd> <target> <quoted>
 *   RUN_HEREDOCS
 *   GET_REDIR_TARGET <cmd_index> <redir_index>
 *   GET_STATUS
 *   DESTROY
 */

#define MAX_CMDS 16
#define MAX_ARGS 32
#define MAX_REDIRS 16

typedef struct s_builder_cmd {
    char *argv[MAX_ARGS];
    int argc;
    t_msh_test_redir_spec redirs[MAX_REDIRS];
    int redir_count;
} t_builder_cmd;

typedef struct s_builder_ctx {
    t_builder_cmd cmds[MAX_CMDS];
    int cmd_count;
    int last_status;
    char *envp[1];  // Empty environment for now
} t_builder_ctx;

static void *g_ctx = NULL;
static t_builder_ctx g_builder = {0};

static void handle_create(int cmd_count, int last_status)
{
    if (cmd_count > MAX_CMDS)
    {
        printf("ERROR: Too many commands (max %d)\n", MAX_CMDS);
        fflush(stdout);
        return;
    }
    
    // Initialize builder
    memset(&g_builder, 0, sizeof(g_builder));
    g_builder.cmd_count = cmd_count;
    g_builder.last_status = last_status;
    g_builder.envp[0] = NULL;
    
    printf("OK\n");
    fflush(stdout);
}

static void handle_add_cmd(const char *args)
{
    int cmd_idx, argc;
    char arg_buf[256];
    const char *ptr = args;
    
    // Parse: cmd_index argc arg1 arg2 ...
    if (sscanf(ptr, "%d %d", &cmd_idx, &argc) != 2)
    {
        printf("ERROR: Invalid ADD_CMD format\n");
        fflush(stdout);
        return;
    }
    
    if (cmd_idx >= g_builder.cmd_count || argc > MAX_ARGS - 1)
    {
        printf("ERROR: Invalid cmd_idx or argc\n");
        fflush(stdout);
        return;
    }
    
    // Skip cmd_idx and argc
    ptr = strchr(ptr, ' ');
    if (ptr) ptr = strchr(ptr + 1, ' ');
    if (!ptr)
    {
        printf("ERROR: Missing arguments\n");
        fflush(stdout);
        return;
    }
    ptr++; // Skip space
    
    // Parse arguments
    g_builder.cmds[cmd_idx].argc = 0;
    for (int i = 0; i < argc && ptr && *ptr; i++)
    {
        // Read next argument (space-separated)
        int len = 0;
        while (ptr[len] && ptr[len] != ' ')
            len++;
        
        if (len > 0)
        {
            strncpy(arg_buf, ptr, len);
            arg_buf[len] = '\0';
            g_builder.cmds[cmd_idx].argv[i] = strdup(arg_buf);
            g_builder.cmds[cmd_idx].argc++;
        }
        
        // Move to next argument
        ptr += len;
        while (*ptr == ' ')
            ptr++;
    }
    
    // NULL-terminate argv
    g_builder.cmds[cmd_idx].argv[g_builder.cmds[cmd_idx].argc] = NULL;
    
    printf("OK\n");
    fflush(stdout);
}

static void handle_add_redir(const char *args)
{
    int cmd_idx, type, fd, quoted;
    char target[256];
    
    // Parse: cmd_index type fd target quoted
    if (sscanf(args, "%d %d %d %s %d", &cmd_idx, &type, &fd, target, &quoted) != 5)
    {
        printf("ERROR: Invalid ADD_REDIR format\n");
        fflush(stdout);
        return;
    }
    
    if (cmd_idx >= g_builder.cmd_count)
    {
        printf("ERROR: Invalid cmd_idx\n");
        fflush(stdout);
        return;
    }
    
    t_builder_cmd *cmd = &g_builder.cmds[cmd_idx];
    if (cmd->redir_count >= MAX_REDIRS - 1)
    {
        printf("ERROR: Too many redirections\n");
        fflush(stdout);
        return;
    }
    
    // Add redirection
    cmd->redirs[cmd->redir_count].type = type;
    cmd->redirs[cmd->redir_count].fd = fd;
    cmd->redirs[cmd->redir_count].target = strdup(target);
    cmd->redirs[cmd->redir_count].quoted = quoted;
    cmd->redir_count++;
    
    // Add sentinel
    cmd->redirs[cmd->redir_count].type = -1;
    cmd->redirs[cmd->redir_count].fd = 0;
    cmd->redirs[cmd->redir_count].target = NULL;
    cmd->redirs[cmd->redir_count].quoted = 0;
    
    printf("OK\n");
    fflush(stdout);
}

static void handle_build_context(void)
{
    if (g_ctx)
    {
        printf("ERROR: Context already built\n");
        fflush(stdout);
        return;
    }
    
    // Build t_msh_test_cmd_spec array
    t_msh_test_cmd_spec *cmd_specs = malloc(sizeof(t_msh_test_cmd_spec) * g_builder.cmd_count);
    if (!cmd_specs)
    {
        printf("ERROR: Memory allocation failed\n");
        fflush(stdout);
        return;
    }
    
    for (int i = 0; i < g_builder.cmd_count; i++)
    {
        cmd_specs[i].argv = (const char **)g_builder.cmds[i].argv;
        cmd_specs[i].redirs = g_builder.cmds[i].redirs;
    }
    
    // Create context
    g_ctx = msh_test_ctx_create(cmd_specs, g_builder.cmd_count, 
                                g_builder.envp, g_builder.last_status);
    
    free(cmd_specs);
    
    if (!g_ctx)
    {
        printf("ERROR: Failed to create context\n");
        fflush(stdout);
        return;
    }
    
    printf("OK\n");
    fflush(stdout);
}

static void handle_run_heredocs(void)
{
    if (!g_ctx)
    {
        printf("ERROR: No context created (use BUILD_CONTEXT first)\n");
        fflush(stdout);
        return;
    }
    
    int result = msh_test_set_here_docs(g_ctx);
    printf("RESULT %d\n", result);
    fflush(stdout);
}

static void handle_get_redir_target(int cmd_idx, int redir_idx)
{
    if (!g_ctx)
    {
        printf("ERROR: No context\n");
        fflush(stdout);
        return;
    }
    
    const char *target = msh_test_get_redir_target(g_ctx, cmd_idx, redir_idx);
    if (target)
        printf("TARGET %s\n", target);
    else
        printf("TARGET_NULL\n");
    fflush(stdout);
}

static void cleanup_builder(void)
{
    for (int i = 0; i < g_builder.cmd_count; i++)
    {
        // Free argv
        for (int j = 0; j < g_builder.cmds[i].argc; j++)
        {
            free(g_builder.cmds[i].argv[j]);
        }
        
        // Free redir targets
        for (int j = 0; j < g_builder.cmds[i].redir_count; j++)
        {
            free((char *)g_builder.cmds[i].redirs[j].target);
        }
    }
    
    memset(&g_builder, 0, sizeof(g_builder));
}

int main(void)
{
    char line[4096];
    
    while (fgets(line, sizeof(line), stdin))
    {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        if (strncmp(line, "CREATE ", 7) == 0)
        {
            int cmd_count, last_status;
            sscanf(line + 7, "%d %d", &cmd_count, &last_status);
            handle_create(cmd_count, last_status);
        }
        else if (strncmp(line, "ADD_CMD ", 8) == 0)
        {
            handle_add_cmd(line + 8);
        }
        else if (strncmp(line, "ADD_REDIR ", 10) == 0)
        {
            handle_add_redir(line + 10);
        }
        else if (strcmp(line, "BUILD_CONTEXT") == 0)
        {
            handle_build_context();
        }
        else if (strcmp(line, "RUN_HEREDOCS") == 0)
        {
            handle_run_heredocs();
        }
        else if (strncmp(line, "GET_REDIR_TARGET ", 17) == 0)
        {
            int cmd_idx, redir_idx;
            sscanf(line + 17, "%d %d", &cmd_idx, &redir_idx);
            handle_get_redir_target(cmd_idx, redir_idx);
        }
        else if (strcmp(line, "DESTROY") == 0)
        {
            if (g_ctx)
            {
                msh_test_ctx_destroy(g_ctx);
                g_ctx = NULL;
            }
            cleanup_builder();
            printf("OK\n");
            fflush(stdout);
        }
        else if (strcmp(line, "EXIT") == 0)
        {
            break;
        }
        else
        {
            printf("ERROR: Unknown command: %s\n", line);
            fflush(stdout);
        }
    }
    
    if (g_ctx)
        msh_test_ctx_destroy(g_ctx);
    cleanup_builder();
    
    return 0;
}