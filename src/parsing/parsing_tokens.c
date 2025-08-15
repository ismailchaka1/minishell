/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_tokens.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:30:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 16:15:25 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_command *handle_word_token(t_command *current, t_command *head, 
                                   t_token *tokens, t_tokenizer *tokenizer)
{
    (void)head; // Suppress unused parameter warning
    // Check for null or empty token value
    if (!tokens->value)
    {
        printf("Error: Token without value\n");
        return NULL;
    }
    
    if (!current)
    {
        current = create_new_command(tokens->value, tokenizer);
        if (!current)
            return NULL;
    }
    else
    {
        if (!add_argument_to_command(current, tokens->value))
            return NULL;
    }
    
    return current;
}

t_command *handle_quoted_token(t_command *current, t_command *head,
                                     t_token *tokens, t_tokenizer *tokenizer)
{
    (void)tokenizer; // Suppress unused parameter warning
    // Handle quoted strings (can be empty)
    char *value = tokens->value ? strdup(tokens->value) : strdup("");
    if (!value)
    {
        printf("Error: Memory allocation failed for quoted string\n");
        return NULL;
    }
    
    if (current)
    {
        // Add as argument (quoted strings can be empty arguments)
        if (!add_arg(current, value))
        {
            printf("Error: Memory allocation failed for argument\n");
            free(value);
            return NULL;
        }
    }
    else
    {
        // Check if this is an empty quoted string as a command
        if (value[0] == '\0')
        {
            printf("Error: Empty quoted string cannot be used as command\n");
            free(value);
            return NULL;
        }
        
        current = malloc(sizeof(t_command));
        if (!current)
        {
            free(value);
            return NULL;
        }
        current->command = value;
        current->args = NULL;
        current->redirects = NULL;
        current->input_file = NULL;
        current->output_file = NULL;
        current->append = false;
        current->heredoc = false;
        current->heredoc_delimiter = NULL;
        current->path = NULL;
        current->next = NULL;

        if (!head)
        {
            head = current;
        }
        else
        {
            // Find the last command and link
            t_command *last = head;
            while (last->next)
                last = last->next;
            last->next = current;
        }
    }
    
    return current;
}

t_command *parse_tokens(t_token *tokens, t_tokenizer *tokenizer)
{
    t_command *head = NULL;
    t_command *current = NULL;

    while (tokens && tokens->type != TOKEN_EOF)
    {
        if (tokens->type == TOKEN_WORD)
        {
            current = handle_word_token(current, head, tokens, tokenizer);
            if (!current)
            {
                free_commands(head);
                return NULL;
            }
            if (!head)
                head = current;
        }
        else if (tokens->type == TOKEN_PIPE)
        {
            current = handle_pipe_token(current, tokens, tokenizer);
            if (!current)
            {
                free_commands(head);
                return NULL;
            }
            tokens = tokens->next; // Skip the command token after pipe
        }
        else if (tokens->type == TOKEN_REDIRECT_IN || 
                  tokens->type == TOKEN_REDIRECT_OUT || 
                  tokens->type == TOKEN_APPEND || 
                  tokens->type == TOKEN_HEREDOC)
        {
            current = handle_redirection_token(current, head, tokens, tokenizer);
            if (!current)
            {
                free_commands(head);
                return NULL;
            }
            if (!head)
                head = current;
            if (tokens->next)
                tokens = tokens->next; // Skip the filename token
        }
        else if (tokens->type == TOKEN_SINGLE_QUOTE || 
                  tokens->type == TOKEN_DOUBLE_QUOTE)
        {
            current = handle_quoted_token(current, head, tokens, tokenizer);
            if (!current)
            {
                free_commands(head);
                return NULL;
            }
            if (!head)
                head = current;
        }
        tokens = tokens->next;
    }
    return head;
}
