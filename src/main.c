#include "../include/minishell.h"


volatile sig_atomic_t exit_status = 0;

int main(int argc, char** argv,char **envp)
{
	t_shell	*minishell;

	if(argc != 0 && argv[1] != NULL)
		return(0);
	if (!init_minishell(&minishell, envp))
		return (MALLOC_ERROR);
	MSH_LOG("t_shell structure initialized by Mario");
	free(minishell);
	return (0);
}
