/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_core.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:30:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 16:15:25 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

bool add_arg(t_command *current, char *arg)
{
    if (!current || !arg)
        return false;
        
    size_t count = 0;

    // Count existing args
    if (current->args)
    {
        while (current->args[count])
            count++;
    }

    // Realloc to add new arg + NULL
    char **new_args = malloc(sizeof(char *) * (count + 2));
    if (!new_args)
    {
        return false; // Return failure status
    }

    for (size_t i = 0; i < count; i++)
        new_args[i] = current->args[i];
    new_args[count] = arg;
    new_args[count + 1] = NULL;

    free(current->args);
    current->args = new_args;
    return true; // Success
}

t_command *create_new_command(char *command_name, t_tokenizer *tokenizer)
{
    t_command *cmd = malloc(sizeof(t_command));
    if (!cmd)
        return NULL;
    
    cmd->command = command_name ? strdup(command_name) : NULL;
    if (command_name && !cmd->command)
    {
        free(cmd);
        return NULL;
    }
    
    cmd->args = NULL;
    cmd->redirects = NULL;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append = false;
    cmd->heredoc = false;
    cmd->heredoc_delimiter = NULL;
    cmd->path = NULL;
    cmd->next = NULL;
    cmd->tokens = tokenizer;
    
    return cmd;
}

bool add_argument_to_command(t_command *current, char *value)
{
    char **new_args = NULL;
    
    if (current->args)
    {
        int count = 0;
        while (current->args[count])
            count++;
        new_args = malloc(sizeof(char *) * (count + 2));
        if (!new_args)
        {
            printf("Error: Memory allocation failed for arguments\n");
            return false;
        }
        for (int i = 0; i < count; i++)
            new_args[i] = current->args[i];
        new_args[count] = strdup(value);
        if (!new_args[count])
        {
            printf("Error: Memory allocation failed for argument\n");
            free(new_args);
            return false;
        }
        new_args[count + 1] = NULL;
        free(current->args);
    }
    else if (current && !current->command)
    {
        current->command = strdup(value);
        return (current->command != NULL);
    }
    else
    {
        new_args = malloc(sizeof(char *) * 2);
        if (!new_args)
        {
            printf("Error: Memory allocation failed for arguments\n");
            return false;
        }
        new_args[0] = strdup(value);
        if (!new_args[0])
        {
            printf("Error: Memory allocation failed for argument\n");
            free(new_args);
            return false;
        }
        new_args[1] = NULL;
    }
    current->args = new_args;
    return true;
}

t_command *handle_pipe_token(t_command *current, t_token *tokens, t_tokenizer *tokenizer)
{
    if (!current)
    {
        printf("Error: Pipe without command\n");
        return NULL;
    }
    
    // Check if there's a valid command after the pipe
    if (tokens->next && 
        (tokens->next->type == TOKEN_WORD || 
         tokens->next->type == TOKEN_SINGLE_QUOTE || 
         tokens->next->type == TOKEN_DOUBLE_QUOTE))
    {
        current->next = create_new_command(tokens->next->value, tokenizer);
        if (!current->next)
            return NULL;
        return current->next;
    }
    else
    {
        printf("Error: Pipe without command\n");
        return NULL;
    }
}
