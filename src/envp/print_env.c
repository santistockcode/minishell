/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_env.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:35:53 by mario             #+#    #+#             */
/*   Updated: 2026/01/16 13:22:58 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void	print_env_list(t_list *env)
{
	t_env	*var;

	while (env)
	{
		var = (t_env *)env->content;
		if (var->value)
			printf("%s=%s\n", var->key, var->value);
		else
			printf("%s=\n", var->key);
		env = env->next;
	}
} 
