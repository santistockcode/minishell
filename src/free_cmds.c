#include "../Libft/include/libft.h"
#include "../include/exec.h"
#include "../include/log.h"
#include <stdio.h>
#include <stdlib.h>

static void	free_redir(void *redir_ptr)
{
	t_redir	*redir;

	redir = (t_redir *)redir_ptr;
	if (!redir)
		return ;
	if (redir->target)
		free(redir->target);
	free(redir);
}

void	free_cmd_struct(void *input)
{
	t_cmd	*cmd;
	char	**argv;

	cmd = (t_cmd *)input;
	if (!cmd)
		return ;
	if (cmd->argv)
	{
		argv = cmd->argv;
		while (*argv)
		{
			free(*argv);
			argv++;
		}
		free(cmd->argv);
		cmd->argv = NULL;
	}
	if (cmd->redirs)
		ft_lstclear(&cmd->redirs, free_redir);
	free(cmd);
}

/*
-> 25  			free(cmd->argv);
   26  		}
   27  		if (cmd->redirs)
   28  			ft_lstclear(&cmd->redirs, free_redir);
(lldb) p *(cmd->argv)
(char *) $33 = 0x0000000000000000
(lldb) p cmd->argv
(char **) $34 = 0x00000000004056e0
(lldb) n
double free or corruption (out)
Process 2845515 stopped
* thread #1, name = 'test_free_cmds', stop reason = signal SIGABRT
	frame #0: 0x00007ffff7d2a9fc libc.so.6`pthread_kill@@GLIBC_2.34 + 300
libc.so.6`pthread_kill@@GLIBC_2.34:
->  0x7ffff7d2a9fc <+300>: movl   %eax, %r13d
	0x7ffff7d2a9ff <+303>: negl   %r13d
	0x7ffff7d2aa02 <+306>: cmpl   $0xfffff000, %eax         ; imm = 0xFFFFF000
	0x7ffff7d2aa07 <+311>: movl   $0x0, %eax
*/
// void free_cmd_struct_b(void *input)
// {

// 	t_cmd *cmd;

// 	cmd = (t_cmd *)input;
// 	if (!cmd)
// 		return ;
// 	if (cmd->argv)
// 	{
// 		while (*cmd->argv)
// 		{
// 			free(*cmd->argv);
// 			cmd->argv++;
// 		}
// 		free(cmd->argv);
// 	}
// 	if (cmd->redirs)
// 		ft_lstclear(&cmd->redirs, free_redir);
// 	free(cmd);
// }

void	free_cmds(t_list *cmd_first)
{
	t_list	*node;

	node = cmd_first;
	ft_lstclear(&node, &free_cmd_struct);
}
