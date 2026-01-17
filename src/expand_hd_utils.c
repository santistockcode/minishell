/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_hd_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 15:11:21 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/09 15:12:58 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

/*
Frees a list of characters.
*/
void	free_aux_list(t_list **lst)
{
	t_list	*current;
	t_list	*next;

	current = *lst;
	while (current != NULL)
	{
		next = current->next;
		free(current);
		current = next;
	}
	*lst = NULL;
}

/*
Converts a list of characters to a dynamically allocated string.
*/
char	*list_to_alloc_string(t_list *chars_list)
{
	char	*complete_string;
	int		size;
	int		i;
	t_list	*current_node;

	size = ft_lstsize(chars_list);
	i = 0;
	complete_string = malloc (sizeof(char) * (size + 1));
	if (!complete_string)
		return (NULL);
	current_node = chars_list;
	while (i < size)
	{
		complete_string[i] = (char)(size_t) current_node->content;
		current_node = current_node->next;
		i++;
	}
	complete_string[i] = '\0';
	return (complete_string);
}

/*
Adds a character to the list.
*/
int	add_char_to_list(char c, t_list **chars_list)
{
	t_list	*new_node;

	new_node = ft_lstnew((void *)(size_t)c);
	if (!new_node)
		return (free_aux_list(chars_list), 0);
	ft_lstadd_back(chars_list, new_node);
	return (1);
}

/*
Adds a string to the list.
*/
int	add_string_to_list(char *str, t_list **chars_list)
{
	int	i;

	i = 0;
	while (str[i] != '\0')
	{
		if (add_char_to_list(str[i], chars_list) == 0)
			return (0);
		i++;
	}
	return (1);
}

/*
If the key is found in the environment list, the corresponding value is
allocated and returned through the result_value pointer. If the key is
not found, result_value is set to NULL.
*/
int	get_env_from_key(char *key, t_list *env, char **res_value)
{
	t_list	*current;
	t_env	*pair;
	char	*original_key;

	current = env;
	original_key = key;
	while (current != NULL)
	{
		pair = (t_env *)current->content;
		if (ft_strncmp(pair->key, key, ft_strlen(key)) == 0)
		{
			*res_value = ft_strdup(pair->value);
			if (!*res_value)
				return (free (original_key), -1);
			return (free (original_key), 0);
		}
		current = current->next;
	}
	free (original_key);
	*res_value = NULL;
	return (0);
}
