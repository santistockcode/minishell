/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_stage_utils.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 19:05:36 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/22 08:39:43 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

void	safe_close_p(int *p)
{
	if (p)
	{
		safe_close(p[0]);
		safe_close(p[1]);
	}
}

void	ft_split_free(char **paths)
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

	dprintf(STDERR_FILENO, "[get_path_envp]: Searching for PATH\n");
	while (env)
	{
		env_var = (t_env *)env->content;
		if (env_var && ft_strncmp(env_var->key, "PATH", 4) == 0)
			return (env_var->value);
		env = env->next;
	}
	return (NULL);
}
