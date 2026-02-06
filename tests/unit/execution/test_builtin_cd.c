#include "../../../include/minishell.h"
#include "../../../include/exec.h"
#include "../../support/third_party/minunit.h"
#include "../../support/c_helpers/test_helpers.h"

/*
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ env |grep PWD
PWD=/home/saalarco/.local/share/Trash/files/delete-me
OLDPWD=/home/saalarco/Dev/minishell
saalarco@c1r9s6:~/.local/share/Trash/files/delete-me$ cd /tmp
saalarco@c1r9s6:/tmp$ env |grep PWD
PWD=/tmp
OLDPWD=/home/saalarco/.local/share/Trash/files/delete-me
saalarco@c1r9s6:/tmp$ unset OLDPWD
saalarco@c1r9s6:/tmp$ cd /etc
saalarco@c1r9s6:/etc$ env |grep PWD
PWD=/etc
(es decir, si existe la variable OLDPWD la actualizo si no, no hago nada)
saalarco@c1r9s6:/etc$ cd /tmp
saalarco@c1r9s6:/tmp$ mkdir -p a/b/c
saalarco@c1r9s6:/tmp$ ps
*/


static int test_cd_and_a_point_do_nothing(void)
{
    return (0);
}


int main(void)
{
    mu_run_test(test_cd_and_a_point_do_nothing);

    mu_summary();
    return 0;
}