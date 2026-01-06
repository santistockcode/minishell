#include <stdlib.h>
#include <string.h>
#include "minunit.h"


#include "../../include/exec.h"
#include "../../include/minishell.h"
#include "../../Libft/include/libft.h"


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
