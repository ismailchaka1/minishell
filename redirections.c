/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 18:55:26 by ichakank          #+#    #+#             */
/*   Updated: 2025/07/25 19:58:38 by ichakank         ###   ########.fr       */
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

int handle_commmand_herdoc(t_redirect *redirect)
{
    int fd;
    char *input;

    fd = open(".heredoc", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
        return (-1);
    printf("Heredoc started, type '%s' to end input\n", redirect->filename);
    while (1)
    {
        input = readline("heredoc > ");
        if (!input) // Handle Ctrl+D
            break;

        if (ft_strncmp(input, redirect->filename, ft_strlen(redirect->filename)) == 0)
        {
            free(input);
            break;
        }

        write(fd, input, strlen(input));
        write(fd, "\n", 1);
        free(input);
    }

    lseek(fd, 0, SEEK_SET); // rewind before dup2
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        close(fd);
        return (-1);
    }

    close(fd);
    unlink(".heredoc"); // cleanup temp file
    return 0;
}


int handle_redirections(t_command *command)
{
    t_redirect *redirect = command->redirects;
    
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
            if (handle_commmand_herdoc(redirect) == -1)
                return -1;
        }
        redirect = redirect->next;
    }
    return 0;
}
