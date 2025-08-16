/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_pipeline_helper.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 17:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/16 13:34:09 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	execute_pipeline_child(t_command *current, t_command *commands,
	int pipefd[2], int prev_pipe_read)
{
	char	**env_array;
	char	**exec_args;

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
	env_array = get_double_env(current->shell);
	exec_args = create_args_array(current);
	handle_child_process_pipeline(current, commands, exec_args, env_array);
	exit(EXIT_SUCCESS);
}

void	execute_pipeline_parent(t_command *current, t_shell *shell,
	pid_t *pids, int *i)
{
	(void)shell;
	pids[(*i)++] = 0;
	if (current->path)
	{
		free(current->path);
		current->path = NULL;
	}
}
