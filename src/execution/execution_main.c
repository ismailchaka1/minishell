/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_main.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 17:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 17:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void execute_single_command(t_command *command, t_shell *shell)
{
    if (!command)
        return;
    if (is_builtin_command(command->command))
    {
        int result = execute_builtin(command, shell, false);
        shell->exit_status = result;
        return;
    }
    
    char **env_array;
    char **exec_args;
    
    if (prepare_execution(command, shell, &env_array, &exec_args) == -1)
        return;
    
    pid_t pid = fork();
    if (pid == 0)
    {
        execute_child_process(command, env_array, exec_args, shell);
        free_exec_requirement(command, env_array, exec_args);
        exit(EXIT_SUCCESS);
    }
    free_exec_requirement(command, env_array, exec_args);
    handle_parent_process(pid, shell, env_array, exec_args);
}

void execute_pipeline(t_command *commands, t_shell *shell)
{
    t_command *current = commands;
    int pipefd[2];
    int prev_pipe_read = STDIN_FILENO;
    int cmd_count = 0;
    
    // Count commands in pipeline
    while (current)
    {
        cmd_count++;
        current = current->next;
    }
    
    // Execute pipeline
    current = commands;
    int i = 0;
    pid_t *pids = malloc(sizeof(pid_t) * cmd_count);
    if (!pids)
    {
        perror("malloc");
        return;
    }
    
    while (current)
    {
        // Create a pipe (except for the last command)
        if (current->next && pipe(pipefd) == -1)
        {
            perror("pipe");
            if (prev_pipe_read != STDIN_FILENO)
                close(prev_pipe_read);
            free(pids);
            return;
        }

        get_paths(current, shell);
        char **env_array = get_double_env(shell);
        char **exec_args = create_args_array(current);
        
        if (!exec_args || !env_array)
        {
            fprintf(stderr, "Failed to create arguments array for command: %s\n", current->command);
            if (env_array)
                free_double_env(env_array);
            if (exec_args)
                free(exec_args);
            if (current->path)
            {
                free(current->path);
                current->path = NULL;
            }
            if (prev_pipe_read != STDIN_FILENO)
                close(prev_pipe_read);
            shell->exit_status = 1;
            current = current->next;
            continue;
        }
        
        // Fork for this command
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            if (prev_pipe_read != STDIN_FILENO)
                close(prev_pipe_read);
            if (current->next)
            {
                close(pipefd[0]);
                close(pipefd[1]);
            }
            free(pids);
            return;
        }
        
        if (pid == 0) // Child process
        {
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            if (prev_pipe_read != STDIN_FILENO)
            {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }
            
            if (current->next)
            {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
                        if (is_builtin_command(current->command))
            {
                // Handle redirections for this command
                if (handle_redirections(current) == -1)
                {
                    free_commands(commands);
                    free_env(shell->env);
                    exit(EXIT_FAILURE);
                }
                
                // Execute builtin without additional redirection handling
                int result = 0;
                if (strcmp(current->command, "cd") == 0)
                {
                    result = builtin_cd(shell, current->args);
                }
                else if (strcmp(current->command, "env") == 0)
                {
                    result = builtin_env(shell);
                }
                else if (strcmp(current->command, "pwd") == 0)
                {
                    result = builtin_pwd(shell);
                }
                else if (strcmp(current->command, "export") == 0)
                {
                    result = builtin_export(shell, current->args);
                }
                else if (strcmp(current->command, "unset") == 0)
                {
                    result = ft_unset(shell, current->args);
                }
                else if (strcmp(current->command, "exit") == 0)
                {
                    result = 0;
                }
                free_commands(commands);
                free_env(shell->env);
                exit(result);
            }
            if (!current->path)
            {
                handle_redirections(current);
                // Don't print "Command not found" for files starting with "." as access error was already printed
                if (current->command[0] != '.')
                {
                    fprintf(stderr, "Command not found: %s\n", current->command);
                }
                if (prev_pipe_read != STDIN_FILENO)
                    close(prev_pipe_read);
                if (current->next)
                {
                    close(pipefd[1]);
                    prev_pipe_read = pipefd[0];
                }
                close(STDIN_FILENO);
                close(STDOUT_FILENO);
                close(STDERR_FILENO);
                free(exec_args);
                free_double_env(env_array);
                free_tokenizer(current->tokens);
                free(pids);
                free_commands(commands);
                free_env(shell->env);
                exit(127);
            }
            
            // Execute external command
            if (handle_redirections(current) == -1)
            {
                free(exec_args);
                free_double_env(env_array);
                free_tokenizer(current->tokens);
                free(pids);
                free_commands(commands);
                free(current->path);
                free_env(shell->env);
                exit(EXIT_FAILURE);
            }
            if (execve(current->path, exec_args, env_array) == -1)
            {
                perror("execve");
                free(current->path);
                free(exec_args);
                free_double_env(env_array);
                free_commands(commands);
                free_env(shell->env);
                exit(EXIT_FAILURE);
            }
            
            // Should never reach here
            free(current->path);
            free(exec_args);
            free_double_env(env_array);
            free_commands(commands);
            free_env(shell->env);
            exit(EXIT_SUCCESS);
        }
        else // Parent process
        {
            // Store child PID
            pids[i++] = pid;
            
            // Clean up memory in parent
            free(exec_args);
            free_double_env(env_array);
            if (current->path)
            {
                free(current->path);
                current->path = NULL;
            }
            
            // Close the previous pipe read end if it's not stdin
            if (prev_pipe_read != STDIN_FILENO)
                close(prev_pipe_read);
            
            // If this isn't the last command, the read end of this pipe becomes
            // the input for the next command
            if (current->next)
            {
                close(pipefd[1]); // Close write end in parent
                prev_pipe_read = pipefd[0];
            }
            
            current = current->next;
        }
    }
    
    // Close the final pipe read end if it exists
    if (prev_pipe_read != STDIN_FILENO)
        close(prev_pipe_read);

    // Wait for all child processes to finish
    int status;
    for (i = 0; i < cmd_count; i++)
    {
        waitpid(pids[i], &status, 0);
        if (i == cmd_count - 1 && WIFEXITED(status))
            shell->exit_status = WEXITSTATUS(status);
    }
    
    free(pids);
    setup_interactive_signals(); // Restore interactive signals
}

void execute_external_command(t_command *commands, t_shell *shell)
{
    if (commands && commands->next)
    {
        execute_pipeline(commands, shell);
    }
    else
    {
        execute_single_command(commands, shell);
    }
}
