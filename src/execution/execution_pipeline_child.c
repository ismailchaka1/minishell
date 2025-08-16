/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_pipeline_child.c                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 17:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 17:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	execute_builtin_in_pipeline(t_command *current, t_shell *shell,
	t_command *commands)
{
	int	result;

	if (handle_redirections(current) == -1)
	{
		free_commands(commands);
		free_env(shell->env);
		exit(EXIT_FAILURE);
	}
	result = 0;
	if (!ft_strncmp(current->command, "cd", 3))
		result = builtin_cd(shell, current->args);
	else if (!ft_strncmp(current->command, "env", 4))
		result = builtin_env(shell);
	else if (!ft_strncmp(current->command, "pwd", 4))
		result = builtin_pwd(shell);
	else if (!ft_strncmp(current->command, "export", 7))
		result = builtin_export(shell, current->args);
	else if (!ft_strncmp(current->command, "unset", 6))
		result = ft_unset(shell, current->args);
	free_commands(commands);
	free_env(shell->env);
	exit(result);
}

static void	execute_external_in_pipeline(t_command *current, char **exec_args,
	char **env_array, t_command *commands)
{
	if (handle_redirections(current) == -1)
	{
		free(exec_args);
		free_double_env(env_array);
		free_commands(commands);
		exit(EXIT_FAILURE);
	}
	if (execve(current->path, exec_args, env_array) == -1)
	{
		perror("execve");
		free(current->path);
		free(exec_args);
		free_double_env(env_array);
		free_commands(commands);
		exit(EXIT_FAILURE);
	}
}

void	handle_child_process_pipeline(t_command *current, t_command *commands,
	char **exec_args, char **env_array)
{
	if (is_builtin_command(current->command))
		execute_builtin_in_pipeline(current, NULL, commands);
	if (!current->path)
	{
		handle_redirections(current);
		if (current->command[0] != '.')
		{
			ft_putstr_fd("Command not found: ", STDERR_FILENO);
			ft_putstr_fd(current->command, STDERR_FILENO);
			ft_putstr_fd("\n", STDERR_FILENO);
		}
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		free(exec_args);
		free_double_env(env_array);
		free_commands(commands);
		exit(127);
	}
	execute_external_in_pipeline(current, exec_args, env_array, commands);
}
