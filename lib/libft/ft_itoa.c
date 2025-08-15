/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 01:00:28 by ichakank          #+#    #+#             */
/*   Updated: 2024/11/06 18:02:01 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	count_digit(long n)
{
	int		count;
	long	number;

	count = 0;
	number = n;
	if (number <= 0)
	{
		count++;
		number *= -1;
	}
	while (number > 0)
	{
		number = number / 10;
		count++;
	}
	return (count);
}

char	*ft_itoa(int n)
{
	char	*str;
	int		i;
	long	number;

	number = (long)n;
	i = count_digit(number) - 1;
	str = malloc((count_digit(number) + 1) * sizeof(char));
	if (!str)
		return (NULL);
	str[count_digit(number)] = '\0';
	if (number < 0)
	{
		str[0] = '-';
		number = -number;
	}
	else if (number == 0)
		str[0] = '0';
	while (number > 0)
	{
		str[i--] = (number % 10) + '0';
		number /= 10;
	}
	return (str);
}
