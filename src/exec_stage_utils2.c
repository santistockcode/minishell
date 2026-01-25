#include "../include/minishell.h"

// stage utils 1
char	*ft_strjoin_prot(char const *str1, char const *str2);
void	safe_close_p(int *p);

// fds utils
void	safe_close_rd_fds(t_list *redirs);

// free cmds
void	free_cmd_struct(void *input);


char *build_path(const char *dir, const char *file)
{
	char *path;
	char *tmp;

	if (!dir || !file)
		return (NULL);
	tmp = ft_strjoin_prot(dir, "/");
	if (!tmp)
		return (NULL);
	path = ft_strjoin_prot(tmp, file);
	if (!path)
	{
		free(tmp);
		return (NULL);
	}
	free(tmp);
	return (path);
}

void free_env_struct_child(void *env)
{
    if (!env) return;
    free(((t_env*)env)->key);
    free(((t_env*)env)->value);
    free(env);
}

void free_shell_child(t_shell *sh)
{
    if (!sh) return;
    ft_lstclear(&sh->env, free_env_struct_child);
    if (sh->last_err_op)
        free(sh->last_err_op);
    free(sh);
}

void safe_close_stage_io(t_stage_io *stage_io)
{
	if (!stage_io) return;
	safe_close_p(&stage_io->in_fd);
	safe_close_p(&stage_io->out_fd);
}

void stage_exit(t_shell *sh, t_cmd *cmd, int *p, int exit_code)
{
	safe_close_p(p);
	safe_close_rd_fds(cmd->redirs);
	safe_close_stage_io(cmd->stage_io);
	free(cmd->stage_io);
	msh_print_last_error(sh);
	msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
	close(sh->save_in);
	close(sh->save_out);
	close(sh->save_err);
	free_shell_child(sh);
	free_cmd_struct(cmd);
	exit(exit_code);
}






