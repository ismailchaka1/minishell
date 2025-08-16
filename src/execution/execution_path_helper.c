/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_path_helper.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/16 13:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/16 13:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*build_full_path(char *path, char *command)
{
	char	*full_path;
	char	*temp_path;

	full_path = ft_strjoin(path, "/");
	if (!full_path)
	{
		perror("ft_strjoin");
		return (NULL);
	}
	temp_path = ft_strjoin(full_path, command);
	free(full_path);
	if (!temp_path)
		perror("ft_strjoin");
	return (temp_path);
}

void	handle_absolute_path(t_command *command, char **paths_orig)
{
	if (access(command->command, F_OK | X_OK) == 0)
	{
		command->path = ft_strdup(command->command);
		free_double_env(paths_orig);
		return ;
	}
	else
	{
		command->path = ft_strdup(command->command);
		free_double_env(paths_orig);
		return ;
	}
}
