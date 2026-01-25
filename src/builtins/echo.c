#include "../../include/minishell.h"

int count_n_flags(char **argv)
{
    int count = 0;
    int i = 1;

    while (argv[i])
    {
        if (argv[i][0] != '-' || argv[i][1] != 'n')
            break;
        int j = 1;
        while (argv[i][j] == 'n')
            j++;
        if (argv[i][j] != '\0')
            break;
        count++;
        i++;
    }
    return count;
}

/*
argv[0] = 'echo'
*/
void	echo(char **argv, int fd)
{
    char    *to_print;
    int     flag;
    int     i;

    flag = 0;
    i = 1;
    if (argv[i] && ft_strncmp(argv[i], "-n", 2) == 0)
        flag = 1;
    i = count_n_flags(argv++);
    while (argv[i])
    {
        to_print = argv[i];
        if (to_print)
        {
            ft_putstr_fd(to_print, fd);
            i++;
            if (argv[i])
                ft_putchar_fd(' ', fd);
        }
        else
            break ;
    }
    if (!flag)
        ft_putchar_fd('\n', fd);
}
