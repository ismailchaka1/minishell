/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap_bonus.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 18:33:52 by ichakank          #+#    #+#             */
/*   Updated: 2024/11/12 18:34:25 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*new_element;
	t_list	*new_list;
	void	*new_content;

	new_list = NULL;
	if (!f || !del)
	{
		return (NULL);
	}
	while (lst != NULL)
	{
		new_content = f(lst->content);
		new_element = ft_lstnew(new_content);
		if (!new_element)
		{
			del(new_content);
			ft_lstclear(&new_list, del);
			return (NULL);
		}
		ft_lstadd_back(&new_list, new_element);
		lst = lst->next;
	}
	return (new_list);
}
/*void	*double_value(void *content)
{
	int	*new_value;

	new_value = malloc(sizeof(int));
	if (new_value)
	{
		*new_value = (*(int *)content) * 2;
	}
	return (new_value);
}

void	delete_content(void *content)
{
	free(content);
}

int	main(void) {
	t_list *list = NULL;
	t_list *new_list;

	for (int i = 1; i <= 3; i++) {
		int *value = malloc(sizeof(int));
		if (value) {
			*value = i;
			ft_lstadd_back(&list, ft_lstnew(value));
		}
	}

	new_list = ft_lstmap(list, double_value, delete_content);

	t_list *current = new_list;
	while (current) {
		printf("%d\n", *(int *)current->content);
		current = current->next;
	}

	ft_lstclear(&list, delete_content);
	ft_lstclear(&new_list, delete_content);

	return (0);
}*/