
static int is_here_doc_del(const char *s)
{
	// match "exit" followed by whitespace or end
	if (!s)
		return 0;
	if (ft_strncmp(s, "exit", 4) != 0)
		return 0;
	const char *p = s + 4;
	// allow no args or whitespace + optional number
	while (*p == ' ' || *p == '\t')
		p++;
	// either end or digits
	if (*p == '\0' || ft_isdigit(*p))
		return 1;
	return 0;
}


void    collect_heredoc(t_shell *sh, t_redir *redir)
{
    // buffer of lines for all the lines with a max that I decide
	sh->last_status = 0;
	sh->should_exit = 0;

	MSH_LOG("Minishell initialized with i = %d", sh->i);

	while (!sh->should_exit)
	{
		char *line = readline("here_doc$ ");

		if (line == NULL)
		{
			// EOF (Ctrl-D): set should_exit and break
			sh->should_exit = 1;
			write(STDOUT_FILENO, "\n", 1); // keep prompt aesthetics
			MSH_LOG("EOF received: should_exit=1");
			break;
		}

		// skip empty lines
		if (*line == '\0')
		{
			free(line);
			continue;
		}

		if (is_delimiter(line))
		{
			// parse optional numeric status: "exit [n]"
			int status = sh->last_status;
			char *p = line + 4;
			while (*p == ' ' || *p == '\t')
				p++;
			if (*p != '\0')
				status = ft_atoi(p);
			sh->last_status = status;
			sh->should_exit = 1; // exit builtin in parent context
			MSH_LOG("exit builtin: status=%d, should_exit=1", sh->last_status);
			free(line);
			break;
		}
		// measure and append line to list
		// append line in list
	}

	// alloc space for all the lines involved
	// str concat them excepto el último salto de línea
	// free list 
	// return complete line perfectly allocated or NULL