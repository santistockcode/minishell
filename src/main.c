#include "../include/minishell.h"

int init_minishell(t_shell *minishell,char **envp)
{

}

int main(int argc, char** argv,char **envp)
{
	t_shell *minishell;

	if(argc != 0 && argv[1] != NULL)
		return(0);
	minishell = malloc(sizeof(t_shell));
	if (!minishell_init(&minishell, envp))
		return (MALLOC_ERROR);
	minishell->i = 10;
	MSH_LOG("Minishell initialized with i = %d", minishell->i);
	while(1)
	{
			
	}
	free(minishell);
	return (0);
}