/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   path_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 19:05:36 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/25 13:46:48 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// better name: build path helpers
#include "../include/minishell.h"

char	*ft_strjoin_prot(char const *str1, char const *str2)
{
	char	*r_join;
	int		i;

	i = 0;
	if (str1 == NULL && str2 == NULL)
		return (NULL);
	r_join = (char *)malloc((ft_strlen(str1)
				+ ft_strlen(str2) + 1) * sizeof(char));
	if (r_join == NULL)
		return (NULL);
	if (str1)
	{
		while (*str1)
			r_join[i++] = *str1++;
	}
	while (*str2)
		r_join[i++] = *str2++;
	r_join[i] = '\0';
	return (r_join);
}

char	*build_path(const char *dir, const char *file)
{
	char	*path;
	char	*tmp;

	if (!dir || !file)
		return (NULL);
	tmp = ft_strjoin_prot(dir, "/");
	if (!tmp)
		return (NULL);
	path = ft_strjoin_prot(tmp, file);
	if (!path)
	{
		free(tmp);
		return (NULL);
	}
	free(tmp);
	return (path);
}

// SPLIT FREEE (but norminette complains so I shortened the name)
void	ft_spfr(char **paths)
{
	char	**tmp;

	if (!paths)
		return ;
	tmp = paths;
	while (*tmp)
	{
		free(*tmp);
		tmp++;
	}
	if (paths)
		free(paths);
}

const char	*get_path_envp(t_list *env)
{
	t_env	*env_var;

	while (env)
	{
		env_var = (t_env *)env->content;
		if (env_var && ft_strncmp(env_var->key, "PATH", 4) == 0)
			return (env_var->value);
		env = env->next;
	}
	return (NULL);
}

char *const	*envp_from_env_list(t_list *env)
{
	t_env	*env_var;
	char	**envp;
	char	*pair;
	int		size;
	int		i;

	size = ft_lstsize(env);
	envp = (char **)malloc(sizeof(char *) * (size + 1));
	if (!envp)
		return (NULL);
	i = 0;
	while (env)
	{
		env_var = (t_env *)env->content;
		if (env_var)
		{
			pair = ft_strjoin_prot(env_var->key, "=");
			envp[i] = ft_strjoin_prot(pair, env_var->value);
			free(pair);
			i++;
		}
		env = env->next;
	}
	envp[i] = NULL;
	return (envp);
}
