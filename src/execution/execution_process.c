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

void	free_exec_requirement(t_command *command, char **env_array,
	char **exec_args)
{
	free(command->path);
	free_double_env(env_array);
	free(exec_args);
}

void	free_parsing(t_command *command, t_shell *shell)
{
	if (!command)
		return ;
	free_tokenizer(command->tokens);
	free_commands(command);
	free_env(shell->env);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

static void	handle_directory_error(t_command *command, char **env_array,
	char **exec_args, t_shell *shell)
{
	int	fd;

	fd = open(command->command, __O_DIRECTORY);
	if (fd != -1)
	{
		ft_putstr_fd("minishell: ", STDOUT_FILENO);
		ft_putstr_fd(command->command, STDOUT_FILENO);
		ft_putstr_fd(": Is a directory\n", STDOUT_FILENO);
		close(fd);
		free_exec_requirement(command, env_array, exec_args);
		free_parsing(command, shell);
		exit(EXIT_FAILURE);
	}
}

void	execute_child_process(t_command *command, char **env_array,
	char **exec_args, t_shell *shell)
{
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
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
	handle_directory_error(command, env_array, exec_args, shell);
	if (execve(command->path, exec_args, env_array) == -1)
		handle_execve_error(command, env_array, exec_args, shell);
}
