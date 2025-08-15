/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_process.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 17:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 17:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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

void execute_child_process(t_command *command, char **env_array, char **exec_args, t_shell *shell)
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
    if (execve(command->path, exec_args, env_array) == -1)
    {
        handle_execve_error(command, env_array, exec_args, shell);
        handle_execve_error_continued(command, env_array, exec_args, shell);
    }
}

int prepare_execution(t_command *command, t_shell *shell, char ***env_array, char ***exec_args)
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

void handle_parent_process(pid_t pid, t_shell *shell, char **env_array, char **exec_args)
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
