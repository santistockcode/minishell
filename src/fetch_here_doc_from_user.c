
/*
void	here_doc(char *limiter, int argc)
{
	pid_t	reader;
	int		fd[2];
	char	*line;

	if (argc < 6)
		usage();
	if (pipe(fd) == -1)
		error();
	reader = fork();
	if (reader == 0)
	{
		close(fd[0]);
		while (get_next_line(&line))
		{
			if (ft_strncmp(line, limiter, ft_strlen(limiter)) == 0)
				exit(EXIT_SUCCESS);
			write(fd[1], line, ft_strlen(line));
		}
	}
	else
	{
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		wait(NULL);
	}
}
*/


char   *fetch_here_doc_from_user(t_shell *sh, const char *delim, int should_expand)
{
    int here_doc_pipe[2];
    if (pipe(here_doc_pipe) == -1)
        return (NULL);

    // while line != delimiter
    while (1)
    {
        // fetch line from user
        char *line = readline("> ");
        if (!line)
            break;

        // if line == delimiter
        if (ft_strncmp(line, delim) == 0)
        {
            free(line);
            break;
        }

        // if should_expand
        if (should_expand)
        {
            MSH_LOG("Should expand here_doc not implemented yet");
            //char *expanded_line = expand_line(line, sh);
            //free(line);
            //line = expanded_line;
        }

        // concat line to here_doc
        here_doc = concat_lines(here_doc, line);
        free(line);
    }

    return (here_doc);
}