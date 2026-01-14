/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_envp.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:35:53 by mario             #+#    #+#             */
/*   Updated: 2026/01/14 23:27:33 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/envp.h"

void	print_envp_list(t_list *env)
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
