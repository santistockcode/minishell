#include <stdlib.h>
#include <string.h>

#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

#include "../../../include/exec.h"
#include "../../../include/minishell.h"
#include "../../../include/syswrap.h"
#include "../../../Libft/include/libft.h"


/*
Actions of exec cmds: 
    - evolve cmds list to manage here_docs (already tested)
    - execute commands (pending)
*/

static int test_empty_pipeline_returns_zero(void)
{
    printf("Test: test_empty_pipeline_returns_zero\n");
    t_shell *sh = (t_shell*)malloc(sizeof(t_shell));
    mu_assert("malloc shell failed", sh != NULL);
    sh->i = 0;
    sh->last_status = 0;
    sh->should_exit = 0;

    t_list *pipe_head = NULL;
    int rc = exec_cmds(sh, pipe_head);
    mu_assert("exec_cmds should return 0 for empty pipeline", rc == 0);
    free(sh);
    return 0;
}

int main(void)
{
    mu_run_test(test_empty_pipeline_returns_zero);
    mu_summary();
}
