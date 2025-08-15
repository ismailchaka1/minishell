/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 14:21:39 by ichakank          #+#    #+#             */
/*   Updated: 2024/11/06 16:41:40 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memmove(void *dst, const void *src, size_t n)
{
	size_t				i;
	unsigned char		*tempdst;
	const unsigned char	*tempsrc;

	tempdst = (unsigned char *)dst;
	tempsrc = (const unsigned char *)src;
	i = 0;
	if (tempdst == NULL && tempsrc == NULL)
		return (NULL);
	if (tempdst == tempsrc || n == 0)
	{
		return (dst);
	}
	if (tempdst < tempsrc)
	{
		ft_memcpy(tempdst, tempsrc, n);
	}
	else
	{
		i = n;
		while (i-- > 0)
			tempdst[i] = tempsrc[i];
	}
	return (dst);
}
