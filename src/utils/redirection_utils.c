/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirection_utils.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/14 00:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/14 00:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_redirect *create_redirect(char *filename, int type)
{
    return create_redirect_with_quotes(filename, type, false);
}

t_redirect *create_redirect_with_quotes(char *filename, int type, bool quoted)
{
    if (!filename)
        return NULL;
        
    t_redirect *redirect = malloc(sizeof(t_redirect));
    if (!redirect)
        return NULL;
        
    redirect->filename = strdup(filename);
    if (!redirect->filename)
    {
        free(redirect);
        return NULL;
    }
    
    redirect->type = type;
    redirect->quoted_delimiter = quoted;
    redirect->next = NULL;
    return redirect;
}

void add_redirect(t_command *command, t_redirect *redirect)
{
    if (!command->redirects)
    {
        command->redirects = redirect;
    }
    else
    {
        t_redirect *current = command->redirects;
        while (current->next)
            current = current->next;
        current->next = redirect;
    }
}

void free_redirects(t_redirect *redirects)
{
    while (redirects)
    {
        t_redirect *next = redirects->next;
        free(redirects->filename);
        free(redirects);
        redirects = next;
    }
}

int handle_standalone_redirections(t_command *command, t_shell *shell)
{
    int result = 0;
    t_redirect *redirect = command->redirects;
    
    while (redirect)
    {
        switch (redirect->type)
        {
            case 0: // Input redirection
                if (handle_input_redirection(redirect->filename) != 0)
                    result = 1;
                break;
            case 1: // Output redirection
                if (handle_output_redirection(redirect->filename, false) != 0)
                    result = 1;
                break;
            case 2: // Append redirection
                if (handle_output_redirection(redirect->filename, true) != 0)
                    result = 1;
                break;
            case 3: // Heredoc
                if (handle_heredoc(redirect->filename, !redirect->quoted_delimiter, shell) != 0)
                    result = 1;
                break;
        }
        redirect = redirect->next;
    }
   
    return result;
}
