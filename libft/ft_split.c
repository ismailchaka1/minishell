/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 21:55:26 by ichakank          #+#    #+#             */
/*   Updated: 2024/11/04 00:15:49 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	countword(char const *s, char c)
{
	size_t	i;
	size_t	count;
	size_t	in_word;

	i = 0;
	count = 0;
	in_word = 0;
	while (s[i] != '\0')
	{
		if (s[i] != c && !in_word)
		{
			in_word = 1;
			count++;
		}
		else if (s[i] == c)
		{
			in_word = 0;
		}
		i++;
	}
	return (count);
}

static void	split_free(char **str, size_t k)
{
	if (str)
	{
		while (k > 0)
			free(str[--k]);
		free(str);
	}
}

static int	word_alloc(char **str, size_t k, size_t t)
{
	str[k] = (char *)malloc((t + 1) * sizeof(char));
	if (!str[k])
		return (split_free(str, k), 0);
	return (1);
}

static void	skip_char(const char *s, size_t *index, char c)
{
	while (s[*index] == c)
	{
		(*index)++;
	}
}

char	**ft_split(char const *s, char c)
{
	size_t	k;
	size_t	i;
	size_t	t;
	char	**str;

	k = 0;
	i = 0;
	if (!s)
		return (NULL);
	str = (char **)malloc((countword(s, c) + 1) * sizeof(char *));
	if (!str)
		return (NULL);
	while (k < countword(s, c))
	{
		skip_char(s, &i, c);
		t = 0;
		while (s[i + t] != c && s[i + (++t)] != '\0')
			;
		if (!word_alloc(str, k, t))
			return (NULL);
		ft_strlcpy(str[k], s + i, t + 1);
		str[k++][t] = '\0';
		i += t;
	}
	return (str[k] = NULL, str);
}
