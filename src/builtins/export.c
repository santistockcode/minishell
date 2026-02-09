/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 15:46:33 by mario             #+#    #+#             */
/*   Updated: 2026/02/09 20:11:26 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void		putstr_fd_err(int n, ...);

/*
 * Get an environment variable by key.
 */
t_env	*env_get(t_list *env, char *key)
{
	t_env	*var;

	while (env)
	{
		var = (t_env *)env->content;
		if (ft_strncmp(var->key, key, ft_strlen(var->key)) == 0)
			return (var);
		env = env->next;
	}
	return (NULL);
}

/*
 * Set an environment variable.
 */
int	env_set(t_list **env, char *var)
{
	t_env	*node;
	t_env	*aux;
	t_list	*new;

	node = init_node(var);
	if (!node)
		return (1);
	aux = env_get(*env, node->key);
	if (aux)
	{
		free(aux->value);
		aux->value = node->value;
		free(node->key);
		free(node);
		return (0);
	}
	new = ft_lstnew(node);
	if (!new)
	{
		free_env(node);
		return (1);
	}
	ft_lstadd_back(env, new);
	return (0);
}

static int	is_valid_export_name(const char *var)
{
	size_t	i;

	if (!var || var[0] == '\0')
		return (0);
	if (var[0] == '=' || (!ft_isalpha(var[0]) && var[0] != '_'))
		return (0);
	i = 1;
	while (var[i] && var[i] != '=')
	{
		if (!ft_isalnum(var[i]) && var[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

// wrap env_set to return 0 on non valid input and print errors
int	export(t_list **env, char *var)
{
	int	result;

	if (!var)
		return (0);
	if (!is_valid_export_name(var))
	{
		putstr_fd_err(4, "minishell: export: `", var,
			"': not a valid identifier\n");
		return (1);
	}
	if (ft_strchr(var, '=') == NULL)
		return (0);
	result = env_set(env, var);
	if (result == 1)
	{
		putstr_fd_err(1, "Export: Error: Memory allocation failed\n");
		return (1);
	}
	return (0);
}

int	wrap_export(t_list **env, char **argv)
{
	int	result;
	int	i;

	i = 1;
	result = 0;
	while (argv[i])
	{
		result = export(env, argv[i]);
		i++;
	}
	return (result);
}
