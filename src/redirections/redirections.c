/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 18:55:26 by ichakank          #+#    #+#             */
/*   Updated: 2025/08/15 21:56:46 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int setup_heredoc_file(void)
{
    int fd;
    
    fd = open(".heredoc", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("heredoc file creation");
        setup_interactive_signals(); // Restore signals
        return -1;
    }
    return fd;
}

int write_heredoc_line(int fd, char *input)
{
    if (write(fd, input, ft_strlen(input)) == -1 ||
        write(fd, "\n", 1) == -1)
    {
        perror("heredoc write");
        return -1;
    }
    return 0;
}

int check_delimiter_match(char *input, t_redirect *redirect)
{
    size_t delimiter_len = ft_strlen(redirect->filename);
    
    return (ft_strlen(input) == delimiter_len && 
            ft_strncmp(input, redirect->filename, delimiter_len) == 0);
}

int read_heredoc_input(int fd, t_redirect *redirect)
{
    char *input;
    int eof_received = 0;
    
    while (1)
    {
        if (g_heredoc_interrupted)
            break;
        
        input = readline("heredoc> ");
        if (!input)
        {
            printf("\n");
            eof_received = 1;
            break;
        }
        if (g_heredoc_interrupted)
        {
            free(input);
            break;
        }
        if (check_delimiter_match(input, redirect))
        {
            free(input);
            break;
        }
        if (write_heredoc_line(fd, input) == -1)
        {
            free(input);
            return -1;
        }
        free(input);
    }
    
    return (g_heredoc_interrupted || eof_received) ? -1 : 0;
}

int finalize_heredoc(int fd)
{
    // Rewind file to beginning for reading
    if (lseek(fd, 0, SEEK_SET) == -1)
    {
        perror("heredoc lseek");
        close(fd);
        unlink(".heredoc");
        setup_interactive_signals(); // Restore signals
        return -1;
    }
    
    // Redirect stdin to read from temporary file
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        perror("heredoc dup2");
        close(fd);
        unlink(".heredoc");
        setup_interactive_signals(); // Restore signals
        return -1;
    }

    close(fd);
    unlink(".heredoc"); // cleanup temp file
    setup_interactive_signals(); // Restore signals
    return 0;
}

int handle_command_heredoc(t_redirect *redirect)
{
    int fd;

    // Setup heredoc signal handling
    setup_heredoc_signals();
    g_heredoc_interrupted = 0;

    fd = setup_heredoc_file();
    if (fd == -1)
        return -1;
    
    if (read_heredoc_input(fd, redirect) == -1)
    {
        close(fd);
        unlink(".heredoc");
        setup_interactive_signals(); // Restore signals
        return -1;
    }

    return finalize_heredoc(fd);
}

// Create heredoc file without redirecting stdin (for preprocessing)
int create_heredoc_file(t_redirect *redirect)
{
    int fd;

    // Setup heredoc signal handling
    setup_heredoc_signals();
    g_heredoc_interrupted = 0;

    fd = setup_heredoc_file();
    if (fd == -1)
        return -1;
    
    if (read_heredoc_input(fd, redirect) == -1)
    {
        close(fd);
        unlink(".heredoc");
        setup_interactive_signals(); // Restore signals
        return -1;
    }

    // Don't redirect stdin, just close the fd and restore signals
    close(fd);
    setup_interactive_signals(); // Restore signals
    return 0;
}

// Apply heredoc redirection to stdin (for execution)
int apply_heredoc_redirection(void)
{
    int fd = open(".heredoc", O_RDONLY);
    if (fd == -1)
    {
        perror("heredoc file open");
        return -1;
    }

    // Redirect stdin to read from heredoc file
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        perror("heredoc dup2");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}


t_redirect *find_last_heredoc(t_command *command)
{
    t_redirect *redirect = command->redirects;
    t_redirect *last_heredoc = NULL;
    
    while (redirect && redirect->type == 3)
    {
        last_heredoc = redirect;
        redirect = redirect->next;
    }
    return last_heredoc;
}

int process_single_redirect(t_redirect *redirect, t_redirect *last_heredoc)
{
    if (redirect->type == 0) // Input redirection
        return handle_command_input_redirection(redirect);
    else if (redirect->type == 1 || redirect->type == 2) // Output or append
        return handle_command_output_redirection(redirect);
    else if (redirect->type == 3) // Heredoc
    {
        if (redirect == last_heredoc)
        {
            // For heredocs, use the pre-created file instead of doing interactive input
            return apply_heredoc_redirection();
        }
        else
        {
            // Skip non-last heredocs (they were handled in preprocessing)
            return 0;
        }
    }
    return 0;
}

int handle_redirections(t_command *command)
{
    t_redirect *redirect = command->redirects;
    t_redirect *last_heredoc = find_last_heredoc(command);
    
    while (redirect)
    {
        if (process_single_redirect(redirect, last_heredoc) == -1)
            return -1;
        redirect = redirect->next;
    }
    return 0;
}
