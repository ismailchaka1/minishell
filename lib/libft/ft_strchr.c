/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/27 16:08:25 by ichakank          #+#    #+#             */
/*   Updated: 2024/11/03 23:51:00 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strchr(const char *s, int c)
{
	int		i;
	char	*p;
	int		len;

	i = 0;
	p = (char *)s;
	len = (int)ft_strlen(s);
	while (i <= len)
	{
		if (p[i] == (unsigned char)c)
			return ((p + i));
		i++;
	}
	return (0);
}
