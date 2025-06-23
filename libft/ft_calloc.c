/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 00:41:41 by ichakank          #+#    #+#             */
/*   Updated: 2024/11/03 23:50:01 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_calloc(size_t count, size_t size)
{
	void	*s1;

	if (count != 0 && size > SIZE_MAX / count)
		return (NULL);
	s1 = malloc(count * size);
	if (!s1)
		return (NULL);
	ft_bzero(s1, size * count);
	return (s1);
}
