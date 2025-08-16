/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_path.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 17:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/16 12:57:28 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	check_absolute_or_relative(t_command *command)
{
	if (ft_strchr(command->command, '/'))
		return (1);
	else if (ft_strchr(command->command, '.'))
		return (1);
	return (0);
}

void	check_path(t_command *command, char **paths)
{
	char	*temp_path;

	while (*paths)
	{
		temp_path = build_full_path(*paths, command->command);
		if (!temp_path)
		{
			free_double_env(paths);
			return ;
		}
		if (access(temp_path, F_OK | X_OK) == 0)
		{
			command->path = temp_path;
			break ;
		}
		free(temp_path);
		paths++;
	}
}

static void	handle_no_path_env(t_command *command)
{
	ft_putstr_fd(command->command, STDERR_FILENO);
	ft_putstr_fd(" : No such file or directory\n", STDERR_FILENO);
}

void	get_paths(t_command *command, t_shell *shell)
{
	char	*path_env;
	char	**paths;
	char	**paths_orig;

	path_env = get_env_value(shell->env, "PATH");
	if (!path_env)
	{
		handle_no_path_env(command);
		return ;
	}
	paths = ft_split(path_env, ':');
	paths_orig = paths;
	if (!paths)
		return (perror("ft_split"));
	if (check_absolute_or_relative(command))
	{
		handle_absolute_path(command, paths_orig);
		return ;
	}
	check_path(command, paths);
	free_double_env(paths_orig);
}
