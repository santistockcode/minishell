#include "../include/minishell.h"
#include "../include/log.h"

int main(int argc, char** argv,char **envp)
{
	t_shell	*minishell;

	setup_signal(); // prior to entering main loop
	if(argc != 0 && argv[1] != NULL)
		return(0);
	if (!init_minishell(&minishell, envp))
		return (MALLOC_ERROR);
	logger("main", "t_shell structure initialized by Mario");
	free(minishell);
	return (0);
}
