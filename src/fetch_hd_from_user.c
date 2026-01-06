#include "../include/minishell.h"
#include "../include/exec.h"
#include "../include/syswrap.h"
#include <readline/readline.h>

/*
FIXME: What should I do if readline fails? 
*/
void	repl_here_doc(t_shell *sh, const char *delim, int should_expand, int fd)
{
	char	*line;
	char	*expanded_line;

	while (1)
	{
		line = readline_wrap("> ");
		if (ft_strncmp(line, delim, ft_strlen(delim)) == 0)
		{
			free(line);
			break ;
		}
		if (should_expand == 1)
		{
			expanded_line = expand_hd((const char *) line, sh);
			if (!expanded_line)
			{
				free(line);
				break ;
			}
			free(line);
			line = expanded_line;
		}
		ft_putendl_fd(line, fd);
		free(line);
	}
}

int	fetch_hd_from_user(t_shell *sh, char **delim,
	int should_expand, int suffix)
{
	int		fd;
	char	*here_doc_name;
	char	*tmp_sfx;

	tmp_sfx = ft_itoa(suffix);
	if (!tmp_sfx)
		return (-1);
	here_doc_name = ft_strjoin(".here_doc_", tmp_sfx);
	if (!here_doc_name)
		return (free(tmp_sfx), -1);
	free(tmp_sfx);
	fd = open_wrap(here_doc_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
	{
		free(here_doc_name);
		return (-1);
	}
	repl_here_doc(sh, *delim, should_expand, fd);
	close_wrap(fd);
	free(*delim);
	*delim = NULL;
	*delim = here_doc_name;
	return (0);
}
