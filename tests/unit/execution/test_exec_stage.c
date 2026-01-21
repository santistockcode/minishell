#include "../../../include/minishell.h"
#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

static int test_msh_path_from_cmd_name_returns_0_on_not_found(void)
{
    return (0);
}

static int test_msh_path_from_cmd_name_returns_found_valid_first_in_list(void)
{
    return (0);
}

static int test_msh_path_from_cmd_name_returns_found_valid_middle_in_list(void)
{
    return (0);
}


static int test_msh_path_from_cmd_name_returns_found_valid_last_in_list(void)
{
    return (0);
}


int main(void)
{
    // msh_path_from_cmd_name
    /*
    Qué queremos probar: 
        - Que si fa*/
    mu_run_test(test_msh_path_from_cmd_name_returns_0_on_not_found);
    mu_run_test(test_msh_path_from_cmd_name_returns_found_valid_first_in_list);
    mu_run_test(test_msh_path_from_cmd_name_returns_found_valid_middle_in_list);
    mu_run_test(test_msh_path_from_cmd_name_returns_found_valid_last_in_list);
    mu_summary();
    return 0;
}