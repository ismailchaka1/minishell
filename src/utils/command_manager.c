/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command_manager.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/14 00:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/14 17:43:19 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void init_shell(t_shell *shell, char **envp)
{
    if (!shell)
        return;
    shell->env = init_env(NULL, envp);
    shell->exit_status = 0;
}

int execute_builtin(t_command *command, t_shell *shell, bool pipe)
{
    int result = 0;
    int saved_stdout = dup(STDOUT_FILENO);
    int saved_stdin = dup(STDIN_FILENO);

    if (saved_stdout == -1 || saved_stdin == -1)
    {
        perror("dup failed");
        if (saved_stdout != -1)
            close(saved_stdout);
        if (saved_stdin != -1)
            close(saved_stdin);
        return -1;
    }

    // Handle redirections
    if (!pipe)
    {
        if (handle_redirections(command) == -1)
        {
            close(saved_stdout);
            close(saved_stdin);
            return -1;
        }
    }
 

    if (strcmp(command->command, "cd") == 0)
    {
        result = builtin_cd(shell, command->args);
    }
    else if (strcmp(command->command, "env") == 0)
    {
        result = builtin_env(shell);
    }
    else if (strcmp(command->command, "pwd") == 0)
    {
        result = builtin_pwd(shell);
    }
    else if (strcmp(command->command, "export") == 0)
    {
        result = builtin_export(shell, command->args);
    }
    else if (strcmp(command->command, "unset") == 0)
    {
        result = ft_unset(shell, command->args);
    }
    else if (strcmp(command->command, "exit") == 0)
    {
        shell->exit_status = -1;
        result = 0;
    }

    // Restore original file descriptors
    dup2(saved_stdout, STDOUT_FILENO);
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdout);
    close(saved_stdin);

    return result;
}

int is_builtin_command(const char *command)
{
    return (strcmp(command, "cd") == 0 || 
            strcmp(command, "env") == 0 || 
            strcmp(command, "pwd") == 0 ||
            strcmp(command, "export") == 0 ||
            strcmp(command, "unset") == 0 ||
            strcmp(command, "exit") == 0);
}

void execute_commands(t_shell *shell, t_command *command)
{
    if (!command->command)
    {
        if (handle_standalone_redirections(command, shell) != 0)
        {
            shell->exit_status = 1;
        }
        else
        {
            shell->exit_status = 0;
        }
        command = command->next;
    }

    execute_external_command(command, shell);
}

void free_commands(t_command *commands)
{
    if (!commands)
        return;
        
    while (commands)
    {
        t_command *next = commands->next;
        
        // Free command string
        if (commands->command)
        {
            free(commands->command);
        }
        
        // Free arguments array
        if (commands->args)
        {
            int i = 0;
            while (commands->args && commands->args[i])
            {
                if (commands->args[i])
                    free(commands->args[i]);
                i++;
            }
            free(commands->args);
        }
        
        // Free file names (only if they exist)
        if (commands->input_file)
            free(commands->input_file);
        if (commands->output_file)
            free(commands->output_file);
        if (commands->heredoc_delimiter)
            free(commands->heredoc_delimiter);
        
        // Free redirections
        if (commands->redirects)
            free_redirects(commands->redirects);
        
        // Free the command structure itself
        free(commands);
        commands = next;
    }
}
