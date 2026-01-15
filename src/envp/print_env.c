/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_env.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:35:53 by mario             #+#    #+#             */
/*   Updated: 2026/01/15 18:43:41 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/env.h"

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
