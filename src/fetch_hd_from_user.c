#include "../include/minishell.h"
#include "../include/exec.h"
#include "../include/syswrap.h"
#include <readline/readline.h>

void	repl_here_doc(t_shell *sh, const char *delim, int should_expand, int fd)
{
	char	*line;
	char	*expanded_line;

	while (1)
	{
		line = readline_wrap("> ");
		if (!line)
			break ;
		// FIXME: esto no funciona con readline
		if (ft_strncmp(line, delim, ft_strlen(delim)) == 0)
		{
			free(line);
			break ;
		}
		if (should_expand == 1)
		{
			MSH_LOG("Expanding here_doc, delimiter: %s", delim);
			expanded_line = expand_hd((const char *) line, sh);
			free(line);
			line = expanded_line;
		}
		ft_putendl_fd(line, fd);
		free(line);
	}
}

char	*fetch_hd_from_user(t_shell *sh, const char *delim,
	int should_expand, int suffix)
{
	int		fd;
	char	*here_doc_name;
	char	*tmp_sfx;

	tmp_sfx = ft_itoa(suffix);
	if (!tmp_sfx)
		return (NULL);
	here_doc_name = ft_strjoin(".here_doc_", tmp_sfx);
	free(tmp_sfx);
	fd = open(here_doc_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
	{
		free(here_doc_name);
		return (NULL);
	}
	repl_here_doc(sh, delim, should_expand, fd);
	close(fd);
	return (here_doc_name);
}
