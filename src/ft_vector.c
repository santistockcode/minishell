/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_vector.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 00:00:53 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/01/18 00:01:13 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void	ft_vector_init(t_vector *vector, size_t elem_size)
{
	vector->elem_size = elem_size;
	vector->size = 0;
	vector->capacity = 0;
	vector->data = NULL;
}

void	ft_vector_free(t_vector *vector)
{
	if (!vector)
		return ;
	if (vector->data)
	{
		free(vector->data);
		vector->data = NULL;
	}
	vector->size = 0;
	vector->capacity = 0;
}

int	ft_vector_push(t_vector *vector, const void *elem)
{
	size_t	new_capacity;
	void	*new_data;

	if (!vector || !elem)
		return (-1);
	if (vector->size >= vector->capacity)
	{
		if (vector->capacity > 0)
			new_capacity = vector->capacity * 2;
		else
			new_capacity = 8;
		new_data = ft_realloc(vector->data,
				vector->capacity * vector->elem_size,
				new_capacity * vector->elem_size);
		if (!new_data)
			return (-1);
		vector->data = new_data;
		vector->capacity = new_capacity;
	}
	ft_memcpy(((unsigned char *) vector->data)
		+ vector->size * vector->elem_size,
		elem, vector->elem_size);
	vector->size += 1;
	return (0);
}

void	*ft_vector_get(t_vector *vector, size_t index)
{
	if (!vector || index >= vector->size)
		return (NULL);
	return (((unsigned char *)vector ->data) + index * vector->elem_size);
}