/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_error.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 17:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 17:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void handle_command_not_found(t_command *command, t_shell *shell)
{
    if (!command->path && get_env_value(shell->env, "PATH"))
    {
        // Don't print "Command not found" for files starting with "." or "/" as access error was already handled
        if (command->command[0] != '.' && command->command[0] != '/')
        {
            printf("Command not found: %s\n", command->command);
        }
        free(command->path);
        command->path = NULL;
        if (command->command && command->command[0] != '\0')
            shell->exit_status = 127; // Command not found
        else
            shell->exit_status = 0; // Empty command
    }
}

void handle_execve_error(t_command *command, char **env_array, char **exec_args, t_shell *shell)
{
    if (errno == EACCES)
    {
        write(2, "minishell: permission denied: ", 30);
        write(2, command->command, strlen(command->command));
        write(2, "\n", 1);
        free_exec_requirement(command, env_array, exec_args);
        free_parsing(command, shell);
        exit(126);
    }
    else if (errno == ENOENT)
    {
        write(2, "minishell: ", 11);
        write(2, command->command, strlen(command->command));
        write(2, ": No such file or directory\n", 28);
        free_exec_requirement(command, env_array, exec_args);
        free_parsing(command, shell);
        exit(127);
    }
}

void handle_execve_error_continued(t_command *command, char **env_array, char **exec_args, t_shell *shell)
{
    if (errno == ENOTDIR)
    {
        write(2, "minishell: ", 11);
        write(2, command->command, strlen(command->command));
        write(2, " : Not a directory\n", 19);
        free_exec_requirement(command, env_array, exec_args);
        free_parsing(command, shell);
        exit(126);
    }
    else
    {
        perror("minishell");
    }
    free_exec_requirement(command, env_array, exec_args);
    exit(126);
}
