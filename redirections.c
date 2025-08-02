/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 18:55:26 by ichakank          #+#    #+#             */
/*   Updated: 2025/08/01 08:43:29 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int handle_command_input_redirection(t_redirect *redirect)
{
    int fd;
    
    if (redirect->type == 0) // Input redirection '<'
    {
        fd = open(redirect->filename, O_RDONLY);
        if (fd == -1)
        {
            perror(redirect->filename);
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) == -1)
        {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }
    return 0;
}

int handle_command_output_redirection(t_redirect *redirect)
{
    int fd;
    
    if (redirect->type == 1) // Output redirection '>'
    {
        fd = open(redirect->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1)
        {
            perror(redirect->filename);
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }
    else if (redirect->type == 2) // Append redirection '>>'
    {
        fd = open(redirect->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd == -1)
        {
            perror(redirect->filename);
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }
    return 0;
}

int handle_command_heredoc(t_redirect *redirect)
{
    int fd;
    char *input;
    size_t delimiter_len;

    fd = open(".heredoc", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("heredoc file creation");
        return (-1);
    }
    
    delimiter_len = ft_strlen(redirect->filename);
    // printf("heredoc> ");
    
    while (1)
    {
        input = readline("heredoc> ");
        if (!input)
        {
            printf("\n");
            break;
        }

        if (ft_strlen(input) == delimiter_len && 
            ft_strncmp(input, redirect->filename, delimiter_len) == 0)
        {
            free(input);
            break;
        }

        // Write line to temporary file
        if (write(fd, input, ft_strlen(input)) == -1 ||
            write(fd, "\n", 1) == -1)
        {
            perror("heredoc write");
            free(input);
            close(fd);
            unlink(".heredoc");
            return (-1);
        }
        free(input);
    }

    // Rewind file to beginning for reading
    if (lseek(fd, 0, SEEK_SET) == -1)
    {
        perror("heredoc lseek");
        close(fd);
        unlink(".heredoc");
        return (-1);
    }
    
    // Redirect stdin to read from temporary file
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        perror("heredoc dup2");
        close(fd);
        unlink(".heredoc");
        return (-1);
    }

    close(fd);
    unlink(".heredoc"); // cleanup temp file
    return (0);
}


int handle_redirections(t_command *command)
{
    t_redirect *redirect = command->redirects;
    t_redirect *last_heredoc = NULL;
    while (redirect && redirect->type == 3)
    {
        last_heredoc = redirect;
        redirect = redirect->next;
    }
    redirect = command->redirects;
    while (redirect)
    {
        if (redirect->type == 0) // Input redirection
        {
            if (handle_command_input_redirection(redirect) == -1)
                return -1;
        }
        else if (redirect->type == 1 || redirect->type == 2) // Output or append redirection
        {
            if (handle_command_output_redirection(redirect) == -1)
                return -1;
        }else if (redirect->type == 3)
        {
            if (redirect == last_heredoc)
            {
                if (handle_command_heredoc(redirect) == -1)
                    return -1;
            }else
            {
                handle_heredoc(redirect->filename, !redirect->quoted_delimiter, NULL);
            }
        }
        redirect = redirect->next;
    }
    return 0;
}
