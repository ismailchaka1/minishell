/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   input.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 21:35:54 by root              #+#    #+#             */
/*   Updated: 2025/08/05 21:43:44 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	handle_command_input_redirection(t_redirect *redirect)
{
	int	fd;

	if (redirect->type == 0)
	{
		fd = open(redirect->filename, O_RDONLY);
		if (fd == -1)
		{
			perror(redirect->filename);
			return (-1);
		}
		if (dup2(fd, STDIN_FILENO) == -1)
		{
			perror("dup2");
			close(fd);
			return (-1);
		}
		close(fd);
	}
	return (0);
}
