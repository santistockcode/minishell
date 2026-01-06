#include "../include/minishell.h"

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

int	add_char_to_list(char c, t_list **chars_list)
{
	t_list	*new_node;

	new_node = ft_lstnew((void *)(size_t)c);
	if (!new_node)
		return (free_aux_list(chars_list), 0);
	ft_lstadd_back(chars_list, new_node);
	return (1);
}

// iterates over a string and adds char by char to list without final 0
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

char	*get_env_from_key(char *key, t_list *env)
{
	t_list	*current;
	t_env	*pair;
	char	*original_key;
	char	*tmp_value;

	current = env;
	original_key = key;
	while (current != NULL)
	{
		pair = (t_env *)current->content;
		if (ft_strncmp(pair->key, key, ft_strlen(key)) == 0)
		{
			tmp_value = ft_strdup(pair->value);
			if (!tmp_value)
				return (free (original_key), NULL);
			return (free (original_key), tmp_value);
		}
		current = current->next;
	}
	free (original_key);
	return (NULL);
}
