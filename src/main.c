#include "../include/minishell.h"

int main(int argc, char** argv)
{
	t_shell *minishell;

	if(argc != 0 && argv[1] != NULL)
		return(0);
	minishell = malloc(sizeof(t_shell));
	minishell->i = 10;
	printf("%i\n", minishell->i);
	printf("%s\n",ft_itoa(minishell->i));
}