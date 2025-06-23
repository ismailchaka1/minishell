/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/27 22:40:38 by ichakank          #+#    #+#             */
/*   Updated: 2024/11/12 18:29:14 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *haystack, const char *needle, size_t len)
{
	size_t	i;
	size_t	j;
	size_t	needlelen;

	i = 0;
	j = 0;
	needlelen = ft_strlen(needle);
	if (!needle || needlelen == 0)
		return ((char *)haystack);
	if (needlelen > len)
		return (NULL);
	while (haystack[i] && len >= i)
	{
		j = 0;
		while (needle[j] && haystack[i + j] == needle[j] && (i + j < len))
			j++;
		if (!needle[j])
			return ((char *)(haystack + i));
		i++;
	}
	return (NULL);
}
