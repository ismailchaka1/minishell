/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 00:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/12 17:14:37 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Global flag to track heredoc interruption
volatile int g_heredoc_interrupted = 0;

/*
** Handle Ctrl-C (SIGINT) in interactive mode
** Displays a new prompt on a new line
*/
static void	sigint_handler(int signum)
{
    (void)signum;
    write(1, "\n", 1);
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

/*
** Handle Ctrl-C (SIGINT) during heredoc input
** Sets flag to interrupt heredoc
*/
static void	sigint_heredoc_handler(int signum)
{
    (void)signum;
    g_heredoc_interrupted = 1;
    write(STDOUT_FILENO, "\n", 1);
    // Removed dangerous rl_done = 1 that was causing segfaults
}

/*
** Configure signals for interactive mode (prompt)
** - SIGINT (Ctrl-C): Display a new prompt
** - SIGQUIT (Ctrl-\): Ignore
*/
void	setup_interactive_signals(void)
{
    struct sigaction	sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    // Handle SIGINT (Ctrl-C)
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);
    
    // Ignore SIGQUIT (Ctrl-\)
    sa.sa_handler = SIG_IGN;
    sigaction(SIGQUIT, &sa, NULL);
}

/*
** Configure signals for heredoc mode
** - SIGINT (Ctrl-C): Interrupt heredoc input
** - SIGQUIT (Ctrl-\): Ignore
*/
void	setup_heredoc_signals(void)
{
    struct sigaction	sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // No SA_RESTART to allow interruption
    
    // Handle SIGINT (Ctrl-C) to interrupt heredoc
    sa.sa_handler = sigint_heredoc_handler;
    sigaction(SIGINT, &sa, NULL);
    
    // Ignore SIGQUIT (Ctrl-\)
    sa.sa_handler = SIG_IGN;
    sigaction(SIGQUIT, &sa, NULL);
}
