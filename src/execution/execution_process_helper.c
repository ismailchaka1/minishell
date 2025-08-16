/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_process_helper.c                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 17:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 17:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	prepare_execution(t_command *command, t_shell *shell, char ***env_array,
	char ***exec_args)
{
	get_paths(command, shell);
	handle_command_not_found(command, shell);
	if (!command->path)
		return (-1);
	*env_array = get_double_env(shell);
	*exec_args = create_args_array(command);
	if (!*exec_args)
	{
		free_double_env(*env_array);
		return (-1);
	}
	return (0);
}

void	handle_parent_process(pid_t pid, t_shell *shell, char **env_array,
	char **exec_args)
{
	int	status;

	(void)env_array;
	(void)exec_args;
	if (pid < 0)
	{
		perror("fork");
		return ;
	}
	waitpid(pid, &status, 0);
	if (WIFEXITED(status))
		shell->exit_status = WEXITSTATUS(status);
}
