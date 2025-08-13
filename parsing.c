/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 15:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/12 17:10:03 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "minishell.h"

static bool add_arg(t_command *current, char *arg)
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

t_command *parse_tokens(t_token *tokens, t_tokenizer *tokenizer)
{
    t_command *head = NULL;
    t_command *current = NULL;

    while (tokens && tokens->type != TOKEN_EOF)
    {
        if (tokens->type == TOKEN_WORD)
        {
            // Check for null or empty token value
            if (!tokens->value)
            {
                printf("Error: Token without value\n");
                free_commands(head);
                return NULL;
            }
            
            if (!current)
            {
                
                current = malloc(sizeof(t_command));
                if (!current)
                {
                    free_commands(head);
                    return NULL;
                }
                current->command = strdup(tokens->value);
                if (!current->command)
                {
                    printf("Error: Memory allocation failed for command\n");
                    free(current);
                    free_commands(head);
                    return NULL;
                }
                current->args = NULL;
                current->redirects = NULL;
                current->input_file = NULL;
                current->output_file = NULL;
                current->append = false;
                current->heredoc = false;
                current->heredoc_delimiter = NULL;
                current->path = NULL;
                current->next = NULL;
                current->tokens = tokenizer;

                if (!head)
                    head = current;
            }
            else
            {
                // Add as argument, even if empty (this is valid for args)
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
                        free_commands(head);
                        return NULL;
                    }
                    for (int i = 0; i < count; i++)
                        new_args[i] = current->args[i];
                    new_args[count] = strdup(tokens->value);
                    if (!new_args[count])
                    {
                        printf("Error: Memory allocation failed for argument\n");
                        free(new_args);
                        free_commands(head);
                        return NULL;
                    }
                    new_args[count + 1] = NULL;
                    free(current->args);
                }
                else if (current && !current->command)
                {
                    // if (!current->command)
                    // {
                        current->command = strdup(tokens->value);
                    // }
                }
                else
                {
                    new_args = malloc(sizeof(char *) * 2);
                    if (!new_args)
                    {
                        printf("Error: Memory allocation failed for arguments\n");
                        free_commands(head);
                        return NULL;
                    }
                    new_args[0] = strdup(tokens->value);
                    if (!new_args[0])
                    {
                        printf("Error: Memory allocation failed for argument\n");
                        free(new_args);
                        free_commands(head);
                        return NULL;
                    }
                    new_args[1] = NULL;
                }
                current->args = new_args;
            }
        }else if (tokens->type == TOKEN_PIPE)
        {
            if (current)
            {
                // Check if there's a valid command after the pipe
                if (tokens->next && 
                    (tokens->next->type == TOKEN_WORD || 
                     tokens->next->type == TOKEN_SINGLE_QUOTE || 
                     tokens->next->type == TOKEN_DOUBLE_QUOTE))
                {
                    // Check for empty command after pipe
                    // if (!tokens->next->value || tokens->next->value[0] == '\0')
                    // {
                    //     printf("Error: Empty command after pipe\n");
                    //     free_commands(head);
                    //     return NULL;
                    // }
                    
                    current->next = malloc(sizeof(t_command));
                    if (!current->next)
                    {
                        free_commands(head);
                        return NULL;
                    }
                    current = current->next;
                    current->command = strdup(tokens->next->value);
                    current->args = NULL;
                    current->redirects = NULL;
                    current->input_file = NULL;
                    current->output_file = NULL;
                    current->append = false;
                    current->heredoc = false;
                    current->heredoc_delimiter = NULL;
                    current->path = NULL;
                    current->next = NULL;
                    current->tokens = tokenizer;
                    tokens = tokens->next;
                }
                else
                {
                    printf("Error: Pipe without command\n");
                    free_commands(head);
                    return NULL;
                }
            }
            else
            {
                printf("Error: Pipe without command\n");
                free_commands(head);
                return NULL;
            }
        }else if (tokens->type == TOKEN_REDIRECT_IN || 
                  tokens->type == TOKEN_REDIRECT_OUT || 
                  tokens->type == TOKEN_APPEND || 
                  tokens->type == TOKEN_HEREDOC)
        {
            // Check if there's a filename after the redirection operator
            if (!tokens->next || tokens->next->type == TOKEN_EOF)
            {
                printf("Error: Missing filename after redirection operator\n");
                free_commands(head);
                return NULL;
            }
            
            // Check if the next token is a valid filename (should be a WORD)
            if (tokens->next->type != TOKEN_WORD && 
                tokens->next->type != TOKEN_SINGLE_QUOTE && 
                tokens->next->type != TOKEN_DOUBLE_QUOTE)
            {
                printf("Error: Expected filename after redirection operator\n");
                free_commands(head);
                return NULL;
            }
            
            // Check if filename value exists and is not empty
            if (!tokens->next->value)
            {
                printf("Error: Null filename after redirection operator\n");
                free_commands(head);
                return NULL;
            }
            
            // Check for empty filename
            if (tokens->next->value[0] == '\0')
            {
                printf("Error: Empty filename after redirection operator\n");
                free_commands(head);
                return NULL;
            }
            
            // Additional validation for filename
            char *filename = tokens->next->value;
            
            // Check for special directory names that should not be used as filenames
            if ((tokens->type == TOKEN_REDIRECT_OUT || tokens->type == TOKEN_APPEND) &&
                (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0))
            {
                printf("Error: %s: Is a directory\n", filename);
                free_commands(head);
                return NULL;
            }
            
            // If no current command exists, create a standalone redirection command
            if (!current)
            {
                current = malloc(sizeof(t_command));
                if (!current)
                {
                    free_commands(head);
                    return NULL;
                }
                current->command = NULL; // No actual command, just redirections
                current->args = NULL;
                current->redirects = NULL;
                current->input_file = NULL;
                current->output_file = NULL;
                current->append = false;
                current->heredoc = false;
                current->heredoc_delimiter = NULL;
                current->path = NULL;
                current->next = NULL;
                current->tokens = tokenizer;
                if (!head)
                    head = current;
            }
            
            t_redirect *redirect = NULL;
            
            if (tokens->type == TOKEN_REDIRECT_IN)
            {
                // Check for multiple input redirections
                if (current->input_file)
                {
                    printf("Error: Multiple input redirections not allowed\n");
                    free_commands(head);
                    return NULL;
                }
                
                redirect = create_redirect(filename, 0);
                if (!redirect)
                {
                    printf("Error: Failed to create input redirection\n");
                    free_commands(head);
                    return NULL;
                }
                current->input_file = strdup(filename);
                if (!current->input_file)
                {
                    printf("Error: Memory allocation failed for input file\n");
                    free(redirect);
                    free_commands(head);
                    return NULL;
                }
            }
            else if (tokens->type == TOKEN_REDIRECT_OUT)
            {
                redirect = create_redirect(filename, 1);
                if (!redirect)
                {
                    printf("Error: Failed to create output redirection\n");
                    free_commands(head);
                    return NULL;
                }
                if (current->output_file)
                    free(current->output_file);
                current->output_file = strdup(filename);
                if (!current->output_file)
                {
                    printf("Error: Memory allocation failed for output file\n");
                    free(redirect);
                    free_commands(head);
                    return NULL;
                }
                current->append = false;
            }
            else if (tokens->type == TOKEN_APPEND)
            {
                redirect = create_redirect(filename, 2);
                if (!redirect)
                {
                    printf("Error: Failed to create append redirection\n");
                    free_commands(head);
                    return NULL;
                }
                if (current->output_file)
                    free(current->output_file);
                current->output_file = strdup(filename);
                if (!current->output_file)
                {
                    printf("Error: Memory allocation failed for append file\n");
                    free(redirect);
                    free_commands(head);
                    return NULL;
                }
                current->append = true;
            }
            else if (tokens->type == TOKEN_HEREDOC)
            {
                bool is_quoted = (tokens->next->type == TOKEN_SINGLE_QUOTE || 
                                 tokens->next->type == TOKEN_DOUBLE_QUOTE);
                redirect = create_redirect_with_quotes(filename, 3, is_quoted);
                if (!redirect)
                {
                    printf("Error: Failed to create heredoc redirection\n");
                    free_commands(head);
                    return NULL;
                }
                current->heredoc = true;
                if (current->heredoc_delimiter)
                    free(current->heredoc_delimiter);
                current->heredoc_delimiter = strdup(filename);
                if (!current->heredoc_delimiter)
                {
                    printf("Error: Memory allocation failed for heredoc delimiter\n");
                    free(redirect);
                    free_commands(head);
                    return NULL;
                }
            }
            
            if (redirect)
                add_redirect(current, redirect);
                
            if (tokens->next)
                tokens = tokens->next;
        }else if (tokens->type == TOKEN_SINGLE_QUOTE || 
                  tokens->type == TOKEN_DOUBLE_QUOTE)
        {
            // Handle quoted strings (can be empty)
            char *value = tokens->value ? strdup(tokens->value) : strdup("");
            if (!value)
            {
                printf("Error: Memory allocation failed for quoted string\n");
                free_commands(head);
                return NULL;
            }
            
            if (current)
            {
                // Add as argument (quoted strings can be empty arguments)
                if (!add_arg(current, value))
                {
                    printf("Error: Memory allocation failed for argument\n");
                    free(value);
                    free_commands(head);
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
                    free_commands(head);
                    return NULL;
                }
                
                current = malloc(sizeof(t_command));
                if (!current)
                {
                    free(value);
                    free_commands(head);
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
                    head = current;
                else
                {
                    // Find the last command and link
                    t_command *last = head;
                    while (last->next)
                        last = last->next;
                    last->next = current;
                }
            }
        }
        tokens = tokens->next;
    }
    return head;
}

void print_commands(t_command *commands)
{
    while (commands)
    {
        printf("Command: %s\n", commands->command);
        if (commands->args)
        {
            printf("  Args: ");
            for (int i = 0; commands->args[i]; i++)
                printf("%d %s|\n", i, commands->args[i]);
            printf("\n");
        }
        
        // Print all redirections
        if (commands->redirects)
        {
            printf("  Redirections:\n");
            t_redirect *redirect = commands->redirects;
            while (redirect)
            {
                printf("    %s -> %s\n", 
                    (redirect->type == 0) ? "Input" :
                    (redirect->type == 1) ? "Output" :
                    (redirect->type == 2) ? "Append" : "Heredoc",
                    redirect->filename);
                redirect = redirect->next;
            }
        }
        
        // Keep backward compatibility output
        if (commands->input_file)
            printf("  Input file: %s\n", commands->input_file);
        if (commands->output_file)
            printf("  Output file: %s\n", commands->output_file);
        if (commands->append)
            printf("  Append mode: true\n");
        if (commands->heredoc)
            printf("  Heredoc: %s\n", commands->heredoc_delimiter);
        commands = commands->next;
    }
}