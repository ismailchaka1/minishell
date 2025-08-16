/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_main.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 17:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/16 12:57:28 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	execute_single_command(t_command *command, t_shell *shell)
{
	char	**env_array;
	char	**exec_args;
	pid_t	pid;
	int		result;

	if (!command)
		return ;
	if (is_builtin_command(command->command))
	{
		result = execute_builtin(command, shell, false);
		shell->exit_status = result;
		return ;
	}
	if (prepare_execution(command, shell, &env_array, &exec_args) == -1)
		return ;
	pid = fork();
	if (pid == 0)
	{
		execute_child_process(command, env_array, exec_args, shell);
		free_exec_requirement(command, env_array, exec_args);
		exit(EXIT_SUCCESS);
	}
	free_exec_requirement(command, env_array, exec_args);
	handle_parent_process(pid, shell, env_array, exec_args);
}

void	execute_external_command(t_command *commands, t_shell *shell)
{
	if (commands && commands->next)
		execute_pipeline(commands, shell);
	else
		execute_single_command(commands, shell);
}
