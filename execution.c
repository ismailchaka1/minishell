/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 14:55:59 by ichakank          #+#    #+#             */
/*   Updated: 2025/08/13 01:13:47 by root             ###   ########.fr       */
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

char **build_double_env(t_shell *shell, char **env_array, int count)
{
    t_env *current = shell->env;
    int i;

    i = 0;
    if (!env_array || count <= 0)
        return NULL;

    while (current && i < count)
    {
        char *key_equals = ft_strjoin(current->key, "=");
        if (!key_equals)
            return (perror("ft_strjoin"), NULL);
        
        env_array[i] = ft_strjoin(key_equals, current->value);
        free(key_equals);
        
        if (!env_array[i])
            return (perror("ft_strjoin"), NULL);
        current = current->next;
        i++;
    }
    env_array[i] = NULL;
    return env_array;
}

char **get_double_env(t_shell *shell)
{
    t_env *current = shell->env;
    int count;
    
    count = 0;
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
    env_array = build_double_env(shell, env_array, count);
    return env_array;
}

char **create_args_array(t_command *command)
{
    int arg_count;
    int i;

    i = 0;
    arg_count = 0;
    if (command->args)
    {
        while (command->args[arg_count])
            arg_count++;
    }
    
    char **exec_args = malloc(sizeof(char *) * (arg_count + 2));
    if (!exec_args)
    {
        perror("malloc");
        return NULL;
    }
    exec_args[0] = command->command;
    while (i < arg_count)
    {
        exec_args[i + 1] = command->args[i];
        i++;
    }
    exec_args[arg_count + 1] = NULL;
    return exec_args;
}

void free_exec_requirement(t_command *command, char **env_array, char **exec_args)
{
    free(command->path);
    free_double_env(env_array);
    free(exec_args);
}

void free_parsing(t_command *command, t_shell *shell)
{
    if (!command)
        return;
    free_tokenizer(command->tokens);
    free_commands(command);
    free_env(shell->env);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

static void handle_command_not_found(t_command *command, t_shell *shell)
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

static void handle_execve_error(t_command *command, char **env_array, char **exec_args, t_shell *shell)
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

static void handle_execve_error_continued(t_command *command, char **env_array, char **exec_args, t_shell *shell)
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

static void execute_child_process(t_command *command, char **env_array, char **exec_args, t_shell *shell)
{
    int fd;
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    
    // Handle input redirection
    if (handle_redirections(command) == -1)
    {
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        free_exec_requirement(command, env_array, exec_args); 
        free_tokenizer(command->tokens);
        free_commands(command);
        free_env(shell->env);
        exit(EXIT_FAILURE);
    }
    if ((fd = open(command->command, __O_DIRECTORY)) != -1)
    {
        printf("minishell: %s: Is a directory\n", command->command);
        close(fd);
        free_exec_requirement(command, env_array, exec_args);
        free_parsing(command, shell);
        exit(EXIT_FAILURE);
    }
    
    // Execute the command
    if (execve(command->path, exec_args, env_array) == -1)
    {
        handle_execve_error(command, env_array, exec_args, shell);
        handle_execve_error_continued(command, env_array, exec_args, shell);
    }
}

static int prepare_execution(t_command *command, t_shell *shell, char ***env_array, char ***exec_args)
{
    get_paths(command, shell);
    handle_command_not_found(command, shell);
    
    if (!command->path)
        return -1;
    
    *env_array = get_double_env(shell);
    *exec_args = create_args_array(command);
    
    if (!*exec_args)
    {
        free_double_env(*env_array);
        return -1;
    }
    
    return 0;
}

static void handle_parent_process(pid_t pid, t_shell *shell, char **env_array, char **exec_args)
{
    (void)env_array; // Unused in parent process
    (void)exec_args; // Unused in parent process
    if (pid < 0)
    {
        perror("fork");
        return;
    }
    
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
    {
        shell->exit_status = WEXITSTATUS(status);
    }
}

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
            // if (current->next)
            // {
            //     close(pipefd[0]);
            //     dup2(pipefd[1], STDOUT_FILENO);
            //     close(pipefd[1]);
            // }
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
                // if (prev_pipe_read != STDIN_FILENO)
                //     close(prev_pipe_read);
                // if (current->next)
                // {
                //     close(pipefd[0]);
                //     close(pipefd[1]);
                // }
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
            
            // Check if this is a builtin command
            
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
    
    // // Close the final pipe read end if it exists
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

