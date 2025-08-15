/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_redirections.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:30:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 16:15:25 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static bool validate_redirection_filename(t_token *tokens)
{
    // Check if there's a filename after the redirection operator
    if (!tokens->next || tokens->next->type == TOKEN_EOF)
    {
        printf("Error: Missing filename after redirection operator\n");
        return false;
    }
    
    // Check if the next token is a valid filename (should be a WORD)
    if (tokens->next->type != TOKEN_WORD && 
        tokens->next->type != TOKEN_SINGLE_QUOTE && 
        tokens->next->type != TOKEN_DOUBLE_QUOTE)
    {
        printf("Error: Expected filename after redirection operator\n");
        return false;
    }
    
    // Check if filename value exists and is not empty
    if (!tokens->next->value)
    {
        printf("Error: Null filename after redirection operator\n");
        return false;
    }
    
    // Check for empty filename
    if (tokens->next->value[0] == '\0')
    {
        printf("Error: Empty filename after redirection operator\n");
        return false;
    }
    
    // Additional validation for filename
    char *filename = tokens->next->value;
    
    // Check for special directory names that should not be used as filenames
    if ((tokens->type == TOKEN_REDIRECT_OUT || tokens->type == TOKEN_APPEND) &&
        (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0))
    {
        printf("Error: %s: Is a directory\n", filename);
        return false;
    }
    
    return true;
}

static bool parse_input_redirection(t_command *current, char *filename)
{
    // Check for multiple input redirections
    if (current->input_file)
    {
        printf("Error: Multiple input redirections not allowed\n");
        return false;
    }
    
    t_redirect *redirect = create_redirect(filename, 0);
    if (!redirect)
    {
        printf("Error: Failed to create input redirection\n");
        return false;
    }
    current->input_file = strdup(filename);
    if (!current->input_file)
    {
        printf("Error: Memory allocation failed for input file\n");
        free(redirect);
        return false;
    }
    
    add_redirect(current, redirect);
    return true;
}

static bool parse_output_redirection(t_command *current, char *filename)
{
    t_redirect *redirect = create_redirect(filename, 1);
    if (!redirect)
    {
        printf("Error: Failed to create output redirection\n");
        return false;
    }
    if (current->output_file)
        free(current->output_file);
    current->output_file = strdup(filename);
    if (!current->output_file)
    {
        printf("Error: Memory allocation failed for output file\n");
        free(redirect);
        return false;
    }
    current->append = false;
    
    add_redirect(current, redirect);
    return true;
}

static bool parse_append_redirection(t_command *current, char *filename)
{
    t_redirect *redirect = create_redirect(filename, 2);
    if (!redirect)
    {
        printf("Error: Failed to create append redirection\n");
        return false;
    }
    if (current->output_file)
        free(current->output_file);
    current->output_file = strdup(filename);
    if (!current->output_file)
    {
        printf("Error: Memory allocation failed for append file\n");
        free(redirect);
        return false;
    }
    current->append = true;
    
    add_redirect(current, redirect);
    return true;
}

static bool parse_heredoc_redirection(t_command *current, t_token *tokens)
{
    char *filename = tokens->next->value;
    bool is_quoted = (tokens->next->type == TOKEN_SINGLE_QUOTE || 
                     tokens->next->type == TOKEN_DOUBLE_QUOTE);
    t_redirect *redirect = create_redirect_with_quotes(filename, 3, is_quoted);
    if (!redirect)
    {
        printf("Error: Failed to create heredoc redirection\n");
        return false;
    }
    current->heredoc = true;
    if (current->heredoc_delimiter)
        free(current->heredoc_delimiter);
    current->heredoc_delimiter = strdup(filename);
    if (!current->heredoc_delimiter)
    {
        printf("Error: Memory allocation failed for heredoc delimiter\n");
        free(redirect);
        return false;
    }
    
    add_redirect(current, redirect);
    return true;
}

t_command *handle_redirection_token(t_command *current, t_command *head,
                                   t_token *tokens, t_tokenizer *tokenizer)
{
    (void)head; // Suppress unused parameter warning
    if (!validate_redirection_filename(tokens))
        return NULL;
    
    char *filename = tokens->next->value;
    
    // If no current command exists, create a standalone redirection command
    if (!current)
    {
        current = create_new_command(NULL, tokenizer);
        if (!current)
            return NULL;
    }
    
    bool success = false;
    if (tokens->type == TOKEN_REDIRECT_IN)
        success = parse_input_redirection(current, filename);
    else if (tokens->type == TOKEN_REDIRECT_OUT)
        success = parse_output_redirection(current, filename);
    else if (tokens->type == TOKEN_APPEND)
        success = parse_append_redirection(current, filename);
    else if (tokens->type == TOKEN_HEREDOC)
        success = parse_heredoc_redirection(current, tokens);
    
    if (!success)
        return NULL;
    
    return current;
}
