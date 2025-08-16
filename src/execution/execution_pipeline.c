/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_pipeline.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 17:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/16 12:46:14 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	count_commands(t_command *commands)
{
	t_command	*current;
	int			cmd_count;

	current = commands;
	cmd_count = 0;
	while (current)
	{
		cmd_count++;
		current = current->next;
	}
	return (cmd_count);
}

void	wait_for_children(pid_t *pids, int cmd_count, t_shell *shell)
{
	int	status;
	int	i;

	i = 0;
	while (i < cmd_count)
	{
		waitpid(pids[i], &status, 0);
		if (i == cmd_count - 1 && WIFEXITED(status))
			shell->exit_status = WEXITSTATUS(status);
		i++;
	}
}

void	execute_pipeline(t_command *commands, t_shell *shell)
{
	int			prev_pipe_read;
	int			cmd_count;
	pid_t		*pids;

	prev_pipe_read = STDIN_FILENO;
	cmd_count = count_commands(commands);
	pids = malloc(sizeof(pid_t) * cmd_count);
	if (!pids)
	{
		perror("malloc");
		return ;
	}
	execute_pipeline_loop(commands, shell, pids, &prev_pipe_read);
	wait_for_children(pids, cmd_count, shell);
	if (prev_pipe_read != STDIN_FILENO)
		close(prev_pipe_read);
	free(pids);
	setup_interactive_signals();
}

void	execute_pipeline_loop(t_command *commands, t_shell *shell,
		pid_t *pids, int *prev_pipe_read)
{
	t_command	*current;
	int			pipefd[2];
	int			i;
	pid_t		pid;

	current = commands;
	i = 0;
	while (current)
	{
		if (current->next && pipe(pipefd) == -1)
		{
			perror("pipe");
			return ;
		}
		get_paths(current, shell);
		pid = fork();
		if (pid == 0)
			execute_pipeline_child(current, commands, pipefd, *prev_pipe_read);
		else
			execute_pipeline_parent(current, shell, pids, &i);
		current = current->next;
	}
}
