/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 14:55:59 by ichakank          #+#    #+#             */
/*   Updated: 2025/06/30 19:10:08 by ichakank         ###   ########.fr       */
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
        fprintf(stderr, "Error: PATH environment variable not set\n");
        return;
    }

    char **paths = ft_split(path_env, ':');
    char **paths_orig = paths; // Store the original pointer for freeing later
    if (!paths)
    {
        perror("ft_split");
        return;
    }
    if (command->command[0] == '/')
    {
        if (access(command->command, F_OK | X_OK) == 0)
            command->path = ft_strdup(command->command);
    } else if (command->command[0] == '.')
    {
        if (access(command->command, F_OK | X_OK) == 0)
            command->path = ft_strdup(command->command);
    }else
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
        free_double_env(paths_orig); // Free using the original pointer
    }
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
    // Check if this is a builtin command
    if (is_builtin_command(command->command))
    {
        // Execute builtin with input/output redirection
        int result = execute_builtin(command, shell, input_fd, output_fd);
        shell->exit_status = result;
        
        // Close file descriptors if needed
        if (input_fd != STDIN_FILENO)
            close(input_fd);
        if (output_fd != STDOUT_FILENO)
            close(output_fd);
            
        return;
    }
    
    pid_t pid;
    
    get_paths(command, shell);
    if (!command->path)
    {
        fprintf(stderr, "Command not found: %s\n", command->command);
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
        if (command->redirects)
        {
            t_redirect *redirect = command->redirects;
            while (redirect)
            {
                if (redirect->type == 1) // Input redirection
                {
                    int output_fd = open(redirect->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (output_fd < 0)
                    {
                        perror("open output redirection");
                    } else
                    {
                        dup2(output_fd, STDOUT_FILENO);
                    }
                }else if (redirect->type == 0) // Output redirection
                {
                    int input_fd = open(redirect->filename, O_RDONLY);
                    if (input_fd < 0)
                    {
                        perror("open input redirection");
                    } else
                    {
                        dup2(input_fd, STDIN_FILENO);
                    }
                }
                redirect = redirect->next;
            }
        }

        // Execute the command
        if (execve(command->path, exec_args, env_array) == -1)
        {
            perror("execve");
            // free(command->path);
            free(exec_args);
            free_double_env(env_array);
            exit(EXIT_FAILURE);
        }
        
        // This code should never be reached if execve is successful
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
    
    // Close file descriptors in parent
    if (input_fd != STDIN_FILENO)
        close(input_fd);
    if (output_fd != STDOUT_FILENO)
        close(output_fd);
    
    // Wait for the command to finish
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
                int result = execute_builtin(current, shell, STDIN_FILENO, STDOUT_FILENO);
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
    // Check if we have multiple commands (a pipeline)
    if (commands->next)
    {
        printf("Executing pipeline of commands...\n");
        execute_pipeline(commands, shell);
    }
    else
    {
        execute_single_command(commands, shell, STDIN_FILENO, STDOUT_FILENO);
    }
}

