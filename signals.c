/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 00:00:00 by root              #+#    #+#             */
/*   Updated: 2025/06/27 00:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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
** Handle Ctrl-C (SIGINT) during command execution
** Just prints a newline
*/
static void	sigint_child_handler(int signum)
{
    (void)signum;
    write(1, "\n", 1);
}

/*
** Configure signals for interactive mode (prompt)
** - SIGINT (Ctrl-C): Display a new prompt
** - SIGQUIT (Ctrl-\): Ignore
*/
void	setup_interactive_signals(void)
{
    struct sigaction	sa;

    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    // Handle SIGINT (Ctrl-C)
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);
    
    // Ignore SIGQUIT (Ctrl-\)
    sa.sa_handler = SIG_IGN;
    sigaction(SIGQUIT, &sa, NULL);
}

/*
** Configure signals for command execution mode
** - SIGINT (Ctrl-C): Print newline
** - SIGQUIT (Ctrl-\): Print "Quit" and core dump
*/
void	setup_execution_signals(void)
{
    struct sigaction	sa;

    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    // Handle SIGINT (Ctrl-C)
    sa.sa_handler = sigint_child_handler;
    sigaction(SIGINT, &sa, NULL);
    
    // Default action for SIGQUIT (Ctrl-\)
    sa.sa_handler = SIG_DFL;
    sigaction(SIGQUIT, &sa, NULL);
}

/*
** Configure signals for heredoc mode
** - SIGINT (Ctrl-C): Exit heredoc
** - SIGQUIT (Ctrl-\): Ignore
*/
void	setup_heredoc_signals(void)
{
    struct sigaction	sa;

    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    // Handle SIGINT (Ctrl-C)
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);
    
    // Ignore SIGQUIT (Ctrl-\)
    sa.sa_handler = SIG_IGN;
    sigaction(SIGQUIT, &sa, NULL);
}