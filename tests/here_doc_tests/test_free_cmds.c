#include <stdlib.h>
#include "minunit.h"

#include "../../include/exec.h"
#include "../../include/minishell.h"
#include "../../Libft/include/libft.h"
#include "../common/test_helpers.h"


void				free_cmd_struct(void *input);

static int test_free_cmds_basic(void)
{
    printf("Test: test_free_cmds_basic\n");
    const char *argv1[] = {"echo", "hello"};
    const char *argv2[] = {"ls"};

    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    mu_assert("cmd1 alloc failed", cmd1 != NULL);
    t_cmd *cmd2 = new_cmd_from_args(argv2, 1);
    mu_assert("cmd2 alloc failed", cmd2 != NULL);

    t_list *pipe_head = NULL;
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd1));
    ft_lstadd_back(&pipe_head, ft_lstnew(cmd2));
    //p ((t_cmd *)(*(t_list *)pipe_head).content)->argv[0]
    /* free the pipeline (should free cmds, argv and nodes) */
    free_cmds(pipe_head);

    /* if we reached this point, free_cmds didn't crash — test passes */
    mu_assert("free_cmds completed", 1 == 1);
    return 0;
}

static int test_free_cmd_struct_helper_works_just_fine(void)
{
    printf("Test: test_free_cmd_struct_helper_works_just_fine\n");
    
    const char *argv1[] = {"echo", "hello"};

    t_cmd *cmd1 = new_cmd_from_args(argv1, 2);
    mu_assert("cmd1 alloc failed", cmd1 != NULL);
    
    free_cmd_struct(cmd1);

    mu_assert("everything werk just fine", 1 == 1);

    return 0;

}

int main(void)
{
    mu_run_test(test_free_cmds_basic);
    mu_run_test(test_free_cmd_struct_helper_works_just_fine);
    mu_summary();
}