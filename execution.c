/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 14:55:59 by ichakank          #+#    #+#             */
/*   Updated: 2025/08/01 09:04:04 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"


void free_double_env(char **env_array)
{
    char **original = env_array;
    
    if (!env_array)
        return;

    while (*env_array)
    {
        free(*env_array);
        env_array++;
    }
    free(original);
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
    char **paths_orig = paths; // Store the original pointer for freeing later
    if (!paths)
    {
        // printf("dasdasdasdasda\n");
        perror("ft_split");
        return;
    }
    
    if (command->command[0] == '/')
    {
        // check if directory or not
        if (access(command->command, F_OK | X_OK) == 0)
            command->path = ft_strdup(command->command);
        free_double_env(paths_orig); // Free the paths array
        return;
    } 
    else if (command->command[0] == '.')
    {
        if (access(command->command, F_OK | X_OK) == 0)
        {
            printf("valid\n");
            command->path = ft_strdup(command->command);
        }
        else
        {
            perror("access");
            free_double_env(paths_orig);
            return;
        }
        free_double_env(paths_orig); // Free the paths array
        return;
    }
    else
    {
        while (*paths)
        {
            char *full_path = ft_strjoin(*paths, "/");
            if (!full_path)
            {
                perror("ft_strjoin");
                free_double_env(paths_orig);
                return;
            }
            char *temp_path = ft_strjoin(full_path, command->command);
            free(full_path);
            if (!temp_path)
            {
                perror("ft_strjoin");
                free_double_env(paths_orig);
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
    
    // Always free the paths array at the end
    free_double_env(paths_orig);
}



char **get_double_env(t_shell *shell)
{
    t_env *current = shell->env;
    int count = 0;
    int i = 0;
    
    while (current)
    {
        count++;
        current = current->next;
    }
    
    char **env_array = malloc(sizeof(char *) * (count + 1));
    if (!env_array)
    {
        perror("malloc");
        return NULL;
    }
    
    current = shell->env;
    while (current && i < count)
    {
        char *key_equals = ft_strjoin(current->key, "=");
        if (!key_equals)
        {
            for (int j = 0; j < i; j++)
                free(env_array[j]);
            free(env_array);
            return NULL;
        }
        
        env_array[i] = ft_strjoin(key_equals, current->value);
        free(key_equals);
        
        if (!env_array[i])
        {
            for (int j = 0; j < i; j++)
                free(env_array[j]);
            free(env_array);
            return NULL;
        }
        
        current = current->next;
        i++;
    }
    env_array[i] = NULL;
    return env_array;
}

// Helper function to create arguments array for a command
char **create_args_array(t_command *command)
{
    int arg_count = 0;
    if (command->args)
    {
        while (command->args[arg_count])
            arg_count++;
    }
    
    char **exec_args = malloc(sizeof(char *) * (arg_count + 2)); // +1 for command, +1 for NULL
    if (!exec_args)
    {
        perror("malloc");
        return NULL;
    }
    
    exec_args[0] = command->command;
    for (int i = 0; i < arg_count; i++)
        exec_args[i + 1] = command->args[i];
    exec_args[arg_count + 1] = NULL;
    
    return exec_args;
}

// Execute a single command with redirected input/output if needed
void execute_single_command(t_command *command, t_shell *shell, int input_fd, int output_fd)
{
    (void)input_fd; // Unused parameter
    (void)output_fd; // Unused parameter
    // Check if this is a builtin command
    if (is_builtin_command(command->command))
    {
        int result = execute_builtin(command, shell);
        shell->exit_status = result;
        return;
    }
    
    pid_t pid;
    
    get_paths(command, shell);
    if (!command->path)
    {
        printf("Command not found: %s\n", command->command);
        free(command->path);
        command->path = NULL;
        if (command->command && command->command[0] != '\0')
            shell->exit_status = 127; // Command not found
        else
            shell->exit_status = 0; // Empty command
        return;
    }
    
    char **env_array = get_double_env(shell);
    char **exec_args = create_args_array(command);
    
    if (!exec_args)
    {
        free_double_env(env_array);
        return;
    }
    
    pid = fork();
    if (pid == 0) // Child process
    {
        // Handle input redirection
        if (handle_redirections(command) == -1)
        {
            exit(EXIT_FAILURE);
        }
        // Execute the command
        if (execve(command->path, exec_args, env_array) == -1)
        {
            perror("execve");
            free(command->path);
            free(exec_args);
            free_double_env(env_array);
            exit(EXIT_FAILURE);
        }
        free(command->path);
        free(exec_args);
        free_double_env(env_array);
        exit(EXIT_SUCCESS);
    }
    else if (pid < 0)
    {
        perror("fork");
        free(exec_args);
        free_double_env(env_array);
        return;
    }
    
    // Parent process
    free(exec_args);
    free_double_env(env_array);
    free(command->path);
    if (input_fd != STDIN_FILENO)
        close(input_fd);
    if (output_fd != STDOUT_FILENO)
        close(output_fd);
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
    {
        shell->exit_status = WEXITSTATUS(status);
    }
}

// Execute a pipeline of commands
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
            
            // Check if this is a builtin command
            if (is_builtin_command(current->command))
            {
                // Handle redirections for this command
                if (handle_redirections(current) == -1)
                {
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
                
                exit(result);
            }
            
            // Execute external command
            get_paths(current, shell);
            char **env_array = get_double_env(shell);
            char **exec_args = create_args_array(current);
            
            if (!current->path)
            {
                fprintf(stderr, "Command not found: %s\n", current->command);
                exit(EXIT_FAILURE);
            }
            if (handle_redirections(current) == -1)
                exit(EXIT_FAILURE);
            if (execve(current->path, exec_args, env_array) == -1)
            {
                perror("execve");
                free(current->path);
                free(exec_args);
                free_double_env(env_array);
                exit(EXIT_FAILURE);
            }
            
            // Should never reach here
            exit(EXIT_SUCCESS);
        }
        else // Parent process
        {
            // Store child PID
            pids[i++] = pid;
            
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
    
    // Wait for all child processes to finish
    int status;
    for (i = 0; i < cmd_count; i++)
    {
        waitpid(pids[i], &status, 0);
        if (i == cmd_count - 1 && WIFEXITED(status))
            shell->exit_status = WEXITSTATUS(status);
    }
    
    free(pids);
}

void execute_external_command(t_command *commands, t_shell *shell)
{
    if (commands->next)
    {
        execute_pipeline(commands, shell);
    }
    else
    {
        execute_single_command(commands, shell, STDIN_FILENO, STDOUT_FILENO);
    }
}

