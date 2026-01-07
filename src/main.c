#include "../include/minishell.h"


int	exit_status = 0;

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
	septup_signal();
	while(1)
	{

	}
	free(minishell);
	return (0);
}