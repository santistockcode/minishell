#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "minunit.h"
#include "../common/test_helpers.h"

#include "../../include/exec.h"
#include "../../include/minishell.h"
#include "../../include/syswrap.h"
#include "../../Libft/include/libft.h"

/*
Helpers to build and free the env list used by expand_hd.
*/
static t_list *make_env_node(const char *key, const char *value)
{
    t_env *pair = (t_env *)malloc(sizeof(t_env));
    if (!pair)
        return NULL;
    pair->key = ft_strdup(key);
    pair->value = ft_strdup(value ? value : "");
    if (!pair->key || !pair->value)
    {
        free(pair->key);
        free(pair->value);
        free(pair);
        return NULL;
    }
    return ft_lstnew(pair);
}

static void env_add(t_list **env, const char *key, const char *value)
{
    t_list *node = make_env_node(key, value);
    if (!node)
        return;
    ft_lstadd_back(env, node);
}

static void env_free(t_list **env)
{
    t_list *cur = *env;
    while (cur)
    {
        t_list *next = cur->next;
        t_env *pair = (t_env *)cur->content;
        if (pair)
        {
            free(pair->key);
            free(pair->value);
            free(pair);
        }
        free(cur);
        cur = next;
    }
    *env = NULL;
}

static int expect_expand(t_list *env, const char *input, const char *expected, int last_status)
{
    t_shell *sh = (t_shell *)malloc(sizeof(t_shell));
    if (!sh)
        return -1;
    sh->env = env;
    sh->last_status = last_status;

    char *in = ft_strdup(input);
    mu_assert("input allocation failed", in != NULL);
    char *out = expand_hd((const char *) in, sh);
    free(in);
    mu_assert("expansion returned NULL", out != NULL);
    mu_assert_strcmp(input, expected, out);
    free(out);
    free(sh);
    return 0;
}

/* Simple text */
static int test_simple_abc(void)
{
    printf("Test: test_simple_abc\n");
    t_list *env = NULL;
    int rc = expect_expand(env, "abc", "abc", 0);
    env_free(&env);
    return rc;
}

static int test_simple_empty(void)
{
    printf("Test: test_simple_empty\n");
    t_list *env = NULL;
    int rc = expect_expand(env, "", "", 0);
    env_free(&env);
    return rc;
}

// /* Variable expansion */
static int test_home_set(void)
{
    printf("Test: test_home_set\n");
    t_list *env = NULL;
    env_add(&env, "HOME", "/Users/tester");
    int rc = expect_expand(env, "$HOME", "/Users/tester", 0);
    env_free(&env);
    return rc;
}


static int test_home_missing(void)
{
    printf("Test: test_home_missing\n");
    t_list *env = NULL;
    /* No HOME in env -> should omit and yield empty string */
    int rc = expect_expand(env, "$HOME", "", 0);
    env_free(&env);
    return rc;
}

/* Special cases */
static int test_status_qmark(void)
{
    printf("Test: test_status_qmark\n");
    t_list *env = NULL;
    /* Provide $? via a special key "?" */
    env_add(&env, "?", "42");
    int rc = expect_expand(env, "$?", "42", 42);
    env_free(&env);
    return rc;
}

static int test_dollar_dollar(void)
{
    printf("Test: test_dollar_dollar\n");
    t_list *env = NULL;
    /* Treat $$ as literal '$' followed by invalid var -> result is "" */
    int rc = expect_expand(env, "$$", "$$", 0);
    env_free(&env);
    return rc;
}

static int test_dollar_odd_number(void)
{
    printf("Test: test_dollar_odd_number\n");
    t_list *env = NULL;
    /* Invalid var name after $ -> keep literal "" */
    int rc = expect_expand(env, "$$$", "$$$", 0);
    env_free(&env);
    return rc;
}

/* Edge cases */
static int test_dollar_space(void)
{
    printf("Test: test_dollar_space\n");
    t_list *env = NULL;
    int rc = expect_expand(env, "$ ", "$ ", 0);
    env_free(&env);
    return rc;
}

static int test_dollar_dash(void)
{
    printf("Test: test_dollar_dash\n");
    t_list *env = NULL;
    /* Invalid var name after $ -> keep literal "" */
    int rc = expect_expand(env, "$-", "", 0);
    env_free(&env);
    return rc;
}

static int test_dollar_at_end(void)
{
    printf("Test: test_dollar_at_end\n");
    t_list *env = NULL;
    int rc = expect_expand(env, "$", "$", 0);
    env_free(&env);
    return rc;
}

static int test_concat_unfound_var(void)
{
    printf("Test: test_concat_unfound_var\n");
    t_list *env = NULL;
    /* Even if HOME exists, var parsed as HOMEdef (alnum run); missing -> removed */
    env_add(&env, "HOME", "/Users/tester");
    int rc = expect_expand(env, "abc$HOMEdef", "abc", 0);
    env_free(&env);
    return rc;
}

/* Multiple variables */
static int test_multi_AB(void)
{
    printf("Test: test_multi_AB\n");
    t_list *env = NULL;
    env_add(&env, "A", "foo");
    env_add(&env, "B", "bar");
    int rc = expect_expand(env, "$A$B", "foobar", 0);
    env_free(&env);
    return rc;
}

static int test_multi_A_text_B(void)
{
    printf("Test: test_multi_A_text_B\n");
    t_list *env = NULL;
    env_add(&env, "A", "alpha");
    env_add(&env, "B", "beta");
    int rc = expect_expand(env, "$A text $B", "alpha text beta", 0);
    env_free(&env);
    return rc;
}

static int test_multi_ABC_all_present(void)
{
    printf("Test: test_multi_ABC_all_present\n");
    t_list *env = NULL;
    env_add(&env, "A", "1");
    env_add(&env, "B", "2");
    env_add(&env, "C", "3");
    int rc = expect_expand(env, "$A$B$C", "123", 0);
    env_free(&env);
    return rc;
}

static int test_multi_some_missing(void)
{
    printf("Test: test_multi_some_missing\n");
    t_list *env = NULL;
    env_add(&env, "A", "X");
    /* B missing */
    env_add(&env, "C", "Z");
    int rc = expect_expand(env, "$A$B$C", "XZ", 0);
    env_free(&env);
    return rc;
}

static int test_multi_none_present(void)
{
    printf("Test: test_multi_none_present\n");
    t_list *env = NULL;
    int rc = expect_expand(env, "$A$B$C", "", 0);
    env_free(&env);
    return rc;
}

/* Additional edge-case tests imitating bash-like here-doc expansion */

static int test_underscore_var(void)
{
    printf("Test: test_underscore_var\n");
    t_list *env = NULL;
    env_add(&env, "MY_VAR", "value");
    int rc = expect_expand(env, "$MY_VAR", "value", 0);
    env_free(&env);
    return rc;
}

static int test_digit_var(void)
{
    printf("Test: test_digit_var\n");
    t_list *env = NULL;
    env_add(&env, "1", "one");
    int rc = expect_expand(env, "$1", "one", 0);
    env_free(&env);
    return rc;
}

static int test_var_with_digits_in_name(void)
{
    printf("Test: test_var_with_digits_in_name\n");
    t_list *env = NULL;
    env_add(&env, "VAR123", "v123");
    int rc = expect_expand(env, "$VAR123", "v123", 0);
    env_free(&env);
    return rc;
}

static int test_var_followed_by_equals(void)
{
    printf("Test: test_var_followed_by_equals\n");
    t_list *env = NULL;
    env_add(&env, "VAR", "val");
    int rc = expect_expand(env, "$VAR=tail", "", 0);
    env_free(&env);
    return rc;
}

static int test_zero_var(void)
{
    printf("Test: test_zero_var\n");
    t_list *env = NULL;
    env_add(&env, "0", "prog");
    int rc = expect_expand(env, "$0", "prog", 0);
    env_free(&env);
    return rc;
}

static int test_long_name_with_underscores_and_digits(void)
{
    printf("Test: test_long_name_with_underscores_and_digits\n");
    t_list *env = NULL;
    env_add(&env, "__X1", "ok");
    int rc = expect_expand(env, "$__X1", "ok", 0);
    env_free(&env);
    return rc;
}

static int test_var_followed_by_colon(void)
{
    printf("Test: test_var_followed_by_colon\n");
    t_list *env = NULL;
    env_add(&env, "X", "val");
    int rc = expect_expand(env, "$X:rest", "", 0);
    env_free(&env);
    return rc;
}

int main(void)
{
    mu_run_test(test_simple_abc);
    mu_run_test(test_simple_empty);

    mu_run_test(test_home_set);
    mu_run_test(test_home_missing);

    mu_run_test(test_status_qmark);
    mu_run_test(test_dollar_dollar);
    mu_run_test(test_dollar_odd_number);

    mu_run_test(test_dollar_space);
    mu_run_test(test_dollar_dash);
    mu_run_test(test_dollar_at_end);
    mu_run_test(test_concat_unfound_var);

    mu_run_test(test_multi_AB);
    mu_run_test(test_multi_A_text_B);
    mu_run_test(test_multi_ABC_all_present);
    mu_run_test(test_multi_some_missing);
    mu_run_test(test_multi_none_present);

    mu_run_test(test_underscore_var);
    mu_run_test(test_digit_var);
    mu_run_test(test_var_followed_by_colon);
    mu_run_test(test_var_followed_by_equals);
    mu_run_test(test_zero_var);
    mu_run_test(test_long_name_with_underscores_and_digits);
    mu_run_test(test_var_with_digits_in_name);

    mu_summary();
    return 0;
}
