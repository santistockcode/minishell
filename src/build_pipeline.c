#include "../include/exec.h"
#include "../include/log.h"
#include "../Libft/include/libft.h"
#include <stdio.h>
#include <stdlib.h>

static void free_redir(void *redir_ptr) {
    t_redir *redir = (t_redir *)redir_ptr;
    if (!redir) return;
    if (redir->target) free(redir->target);
    free(redir);
}

static void free_cmd(t_cmd *cmd) {
	if (!cmd) return;
	if (cmd->argv) {
		for (size_t i = 0; cmd->argv[i]; ++i) free(cmd->argv[i]);
		free(cmd->argv);
	}
	if (cmd->redirs) ft_lstclear(&cmd->redirs, free_redir);
	free(cmd);
}

int exec_pipeline(t_shell *sh, t_list *cmd_first)
{
	int idx = 0;
	for (t_list *node = cmd_first; node; node = node->next) {
		t_cmd *cmd = (t_cmd*)node->content;
		if (!cmd || !cmd->argv) continue;
		printf("CMD[%d]:", idx);
		for (size_t i = 0; cmd->argv[i]; ++i) {
			printf(" %s", cmd->argv[i]);
		}
		printf("\n");
		idx++;
	}
	MSH_LOG("Printed %d commands", idx);
	if (sh) sh->last_status = 0;
	return 0;
}

void free_pipeline(t_list *cmd_first)
{
	t_list *node = cmd_first;
	while (node) {
		t_list *next = node->next;
		free_cmd((t_cmd*)node->content);
		free(node);
		node = next;
	}
}
