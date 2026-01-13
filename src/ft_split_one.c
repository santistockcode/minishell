/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split_one.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:55:08 by mario             #+#    #+#             */
/*   Updated: 2026/01/13 15:35:00 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

static void	ft_dfree(char **tab, int i)
{
	while (i)
	{
		free(tab[i - 1]);
		i--;
	}
	free(tab);
}

static int	ft_numbword(char const *s, char c)
{
	int	i;
	int	numbword;

	i = 0;
	numbword = 0;
	while (s[i] != '\0')
	{
		if (s[i] != c)
		{
			numbword++;
			while (s[i] != c && s[i] != '\0')
				i++;
		}
		else
			i++;
	}
	return (numbword);
}

char	**ft_split(char const *s, char c, int limit)
{
	int		lword;
	int		i;
	char	**tab;

	i = -1;
	tab = (char **)ft_calloc((ft_numbword(s, c) + 1), sizeof(char *));
	if (!tab || !s)
		return (NULL);
	while (*s)
	{
		while (*s == c && *s)
			s++;
		if (*s)
		{
			if (!ft_strchr(s, c))
				lword = ft_strlen(s);
			else
				lword = ft_strchr(s, c) - s;
			tab[++i] = ft_substr(s, 0, lword);
			if (!tab[i])
				return (ft_dfree(tab, i), NULL);
			s += lword;
		}
	}
	return (tab);
}