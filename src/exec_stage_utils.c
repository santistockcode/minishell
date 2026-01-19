/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_stage_utils.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 19:05:36 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/19 19:44:52 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

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
