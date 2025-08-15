/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file_operations.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/14 00:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/14 00:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <fcntl.h>
#include <sys/stat.h>

static int validate_filename(char *filename, bool is_output)
{
    // Check for empty filename
    if (!filename || filename[0] == '\0')
    {
        printf("Error: Empty filename for %s redirection\n", 
               is_output ? "output" : "input");
        return 1;
    }
    
    // Check for special directory names
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
    {
        printf("Error: %s: Is a directory\n", filename);
        return 1;
    }
    
    // Check if filename is a directory
    struct stat file_stat;
    if (stat(filename, &file_stat) == 0 && S_ISDIR(file_stat.st_mode))
    {
        printf("Error: %s: Is a directory\n", filename);
        return 1;
    }
    
    return 0;
}

static int create_file_descriptor(char *filename, int flags, mode_t mode)
{
    int fd = open(filename, flags, mode);
    if (fd == -1)
    {
        perror(filename);
        return -1;
    }
    return fd;
}
#include <fcntl.h>
#include <sys/stat.h>

int handle_output_redirection(char *filename, bool append)
{
    // Validate filename first
    if (validate_filename(filename, true) != 0)
        return 1;
    
    int flags = O_WRONLY | O_CREAT;
    if (append)
        flags |= O_APPEND;
    else
        flags |= O_TRUNC;
    
    int fd = create_file_descriptor(filename, flags, 0644);
    if (fd == -1)
        return 1;
    
    if (append)
        printf("File '%s' opened for appending (or created if it didn't exist)\n", filename);
    else
        printf("File '%s' created/truncated successfully\n", filename);
    
    close(fd);
    return 0;
}

int handle_input_redirection(char *filename)
{
    // Validate filename first
    if (validate_filename(filename, false) != 0)
        return 1;
    
    int fd = create_file_descriptor(filename, O_RDONLY, 0);
    if (fd == -1)
        return 1;
    
    printf("File '%s' opened for reading successfully\n", filename);
    close(fd);
    return 0;
}

int handle_heredoc(char *delimiter, bool expand_vars, t_shell *shell)
{
    (void)shell;
    (void) expand_vars;
    int fd;
    char *input;
    
    // Setup heredoc signal handling
    setup_heredoc_signals();
    g_heredoc_interrupted = 0;
    
    fd = open(".heredoc", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("heredoc file creation");
        setup_interactive_signals();
        return -1;
    }
    
    while (1)
    {
        // Check for interruption before readline
        if (g_heredoc_interrupted)
            break;
            
        input = readline("heredoc > ");
        
        // Check for EOF (Ctrl-D)
        if (!input)
        {
            printf("\n");
            break;
        }
        
        // Check for interruption after readline
        if (g_heredoc_interrupted)
        {
            free(input);
            break;
        }
        
        if (*input)
        {
            size_t input_len = ft_strlen(input);
            size_t delimiter_len = ft_strlen(delimiter);
            
            if (input_len == delimiter_len && 
                ft_strncmp(input, delimiter, delimiter_len) == 0)
            {
                free(input);
                break;
            }
            write(fd, input, input_len);
            write(fd, "\n", 1);
        }
        free(input);
    }
    
    close(fd);
    unlink(".heredoc");
    setup_interactive_signals(); // Restore signals
    
    // Return -1 if interrupted, 0 if completed normally
    return (g_heredoc_interrupted ? -1 : 0);
}
