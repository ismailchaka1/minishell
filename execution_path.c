/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_path.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 17:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 17:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int check_absolute_or_relative(t_command *command)
{
    if (ft_strchr(command->command, '/'))
        return 1; // Absolute path
    else if (ft_strchr(command->command, '.'))
        return 1;
    return 0;
}

void check_path(t_command *command, char **paths)
{
    while (*paths)
    {
        char *full_path = ft_strjoin(*paths, "/");
        if (!full_path)
        {
            perror("ft_strjoin");
            free_double_env(paths);
            return;
        }
        char *temp_path = ft_strjoin(full_path, command->command);
        free(full_path);
        if (!temp_path)
        {
            perror("ft_strjoin");
            free_double_env(paths);
            return;
        }
        if (access(temp_path, F_OK | X_OK) == 0)
        {
            command->path = temp_path;
            break;
        }
        free(temp_path);
        paths++;
    }
}

void get_paths(t_command *command, t_shell *shell)
{
    char *path_env = get_env_value(shell->env, "PATH");
    if (!path_env)
    {
        fprintf(stderr, "%s : No such file or directory\n", command->command);
        return;
    }

    char **paths = ft_split(path_env, ':');
    char **paths_orig = paths;
    if (!paths)
        return (perror("ft_split"));
    if (check_absolute_or_relative(command))
    {
        if (access(command->command, F_OK | X_OK) == 0)
        {
            command->path = ft_strdup(command->command);
            free_double_env(paths_orig);
            return;
        }
        else
        {
            command->path = ft_strdup(command->command);
            // Print access error for absolute paths (starting with / or .)
            // perror("access");
            free_double_env(paths_orig);
            return;
        }
    }
    check_path(command, paths);
    free_double_env(paths_orig);
}
