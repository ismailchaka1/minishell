/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer_main.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:45:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 16:45:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Main tokenization function
bool tokenize(t_tokenizer *tokenizer, t_shell *shell)
{
    while (tokenizer->input[tokenizer->pos])
    {
        // Skip whitespace
        while (isspace(tokenizer->input[tokenizer->pos]))
            tokenizer->pos++;
        
        if (!tokenizer->input[tokenizer->pos])
            break;

        char c = tokenizer->input[tokenizer->pos];
        t_token *token = NULL;

        // Handle operators
        if (c == '|')
        {
            token = create_token(TOKEN_PIPE, "|");
            tokenizer->pos++;
        }
        else if (c == '<')
        {
            if (tokenizer->input[tokenizer->pos + 1] == '<')
            {
                token = create_token(TOKEN_HEREDOC, "<<");
                tokenizer->pos += 2;
            }
            else
            {
                token = create_token(TOKEN_REDIRECT_IN, "<");
                tokenizer->pos++;
            }
        }
        else if (c == '>')
        {
            if (tokenizer->input[tokenizer->pos + 1] == '>')
            {
                token = create_token(TOKEN_APPEND, ">>");
                tokenizer->pos += 2;
            }
            else
            {
                token = create_token(TOKEN_REDIRECT_OUT, ">");
                tokenizer->pos++;
            }
        }
        // Handle quotes and words (with concatenation)
        else if (c == '\'' || c == '"' || !isspace(c))
        {
            // Check if the last token was TOKEN_HEREDOC to handle delimiter quoting
            bool after_heredoc = false;
            if (tokenizer->tokens)
            {
                t_token *last = tokenizer->tokens;
                while (last->next)
                    last = last->next;
                after_heredoc = (last->type == TOKEN_HEREDOC);
            }
            
            // If we're after a heredoc and the current character is a quote,
            // treat it as a quoted delimiter (don't concatenate)
            if (after_heredoc && (c == '\'' || c == '"'))
            {
                char *value = extract_quoted(tokenizer, c);
                if (!value)
                    return false;
                token = create_token((c == '\'') ? TOKEN_SINGLE_QUOTE : TOKEN_DOUBLE_QUOTE, value);
                free(value);
            }
            else
            {
                // Check if this is a standalone variable that should be word-split
                bool is_standalone_var = false;
                if (c == '$' && !after_heredoc)
                {
                    // Look ahead to see if this is just a variable
                    size_t temp_pos = tokenizer->pos + 1;
                    
                    if (tokenizer->input[temp_pos] == '{')
                    {
                        // ${VAR} format
                        temp_pos++;
                        while (tokenizer->input[temp_pos] && tokenizer->input[temp_pos] != '}')
                            temp_pos++;
                        if (tokenizer->input[temp_pos] == '}')
                            temp_pos++;
                    }
                    else if (ft_isalpha(tokenizer->input[temp_pos]) || tokenizer->input[temp_pos] == '_')
                    {
                        // $VAR format
                        while (ft_isalnum(tokenizer->input[temp_pos]) || tokenizer->input[temp_pos] == '_')
                            temp_pos++;
                    }
                    
                    // Check if this variable is followed by whitespace or operator (standalone)
                    if (isspace(tokenizer->input[temp_pos]) || 
                        is_operator(tokenizer->input[temp_pos]) || 
                        tokenizer->input[temp_pos] == '\0')
                    {
                        is_standalone_var = true;
                    }
                }
                
                if (is_standalone_var)
                {
                    // Extract and expand the variable, then split it
                    size_t start = tokenizer->pos;
                    tokenizer->pos++; // Skip $
                    
                    if (tokenizer->input[tokenizer->pos] == '{')
                    {
                        // ${VAR} format
                        tokenizer->pos++;
                        while (tokenizer->input[tokenizer->pos] && tokenizer->input[tokenizer->pos] != '}')
                            tokenizer->pos++;
                        if (tokenizer->input[tokenizer->pos] == '}')
                            tokenizer->pos++;
                    }
                    else
                    {
                        // $VAR format
                        while (ft_isalnum(tokenizer->input[tokenizer->pos]) || tokenizer->input[tokenizer->pos] == '_')
                            tokenizer->pos++;
                    }
                    
                    size_t len = tokenizer->pos - start;
                    char *var_str = ft_substr(tokenizer->input, start, len);
                    if (!var_str)
                        return false;
                    
                    char *expanded = expand_variables(var_str, shell);
                    free(var_str);
                    
                    if (expanded)
                    {
                        // Split and add the tokens
                        if (!add_split_tokens(tokenizer, expanded))
                        {
                            free(expanded);
                            return false;
                        }
                        free(expanded);
                        continue; // Skip the normal token creation
                    }
                    else
                    {
                        // If expansion failed, create empty token
                        token = create_token(TOKEN_WORD, "");
                    }
                }
                else
                {
                    // Normal concatenation logic
                    char *concatenated = ft_strdup("");
                    if (!concatenated)
                        return false;
                    
                    while (tokenizer->input[tokenizer->pos] && 
                           !isspace(tokenizer->input[tokenizer->pos]) && 
                           !is_operator(tokenizer->input[tokenizer->pos]))
                    {
                        char current_char = tokenizer->input[tokenizer->pos];
                        char *part = NULL;
                        
                        if (current_char == '\'')
                        {
                            part = extract_quoted(tokenizer, current_char);
                            if (!part)
                            {
                                free(concatenated);
                                return false;
                            }
                        }
                        else if (current_char == '"')
                        {
                            part = extract_quoted(tokenizer, current_char);
                            if (!part)
                            {
                                free(concatenated);
                                return false;
                            }
                            // Expand variables in double quotes
                            if (ft_strchr(part, '$'))
                            {
                                char *expanded = expand_variables(part, shell);
                                if (expanded)
                                {
                                    free(part);
                                    part = expanded;
                                }
                                // If expansion fails, keep the original part
                            }
                        }
                        else if (current_char == '\\' && tokenizer->input[tokenizer->pos + 1] == '\'')
                        {
                            // Handle escaped single quote outside of quotes
                            part = ft_strdup("'");
                            if (!part)
                            {
                                free(concatenated);
                                return false;
                            }
                            tokenizer->pos += 2; // Skip \' 
                        }
                        else
                        {
                            // Extract unquoted part until next quote or separator
                            size_t start = tokenizer->pos;
                            while (tokenizer->input[tokenizer->pos] &&
                                   !isspace(tokenizer->input[tokenizer->pos]) &&
                                   !is_operator(tokenizer->input[tokenizer->pos]) &&
                                   tokenizer->input[tokenizer->pos] != '\'' &&
                                   tokenizer->input[tokenizer->pos] != '"' &&
                                   !(tokenizer->input[tokenizer->pos] == '\\' && 
                                     tokenizer->input[tokenizer->pos + 1] == '\''))
                            {
                                tokenizer->pos++;
                            }
                            
                            if (tokenizer->pos > start)
                            {
                                size_t len = tokenizer->pos - start;
                                part = ft_substr(tokenizer->input, start, len);
                                if (!part)
                                {
                                    free(concatenated);
                                    return false;
                                }
                                
                                // Expand variables in unquoted part (but not for heredoc delimiters)
                                // Don't split here since we're in concatenation mode
                                if (ft_strchr(part, '$') && !after_heredoc)
                                {
                                    char *expanded = expand_variables(part, shell);
                                    if (expanded)
                                    {
                                        free(part);
                                        part = expanded;
                                    }
                                    // If expansion fails, keep the original part
                                }
                            }
                        }
                        
                        if (part)
                        {
                            char *temp = ft_strjoin(concatenated, part);
                            free(concatenated);
                            free(part);
                            concatenated = temp;
                            if (!concatenated)
                                return false;
                        }
                    }
                    
                    // Create appropriate token type based on content
                    if (concatenated && concatenated[0] != '\0')
                    {
                        token = create_token(TOKEN_WORD, concatenated);
                    }
                    else if (concatenated)
                    {
                        // Empty string from variable expansion - create empty word token
                        token = create_token(TOKEN_WORD, "");
                    }
                    
                    free(concatenated);
                }
            }
        }

        if (token)
            add_token(tokenizer, token);
        else
        {
            printf("Error: Invalid token at position %zu\n", tokenizer->pos);
            return false;
        }
    }

    // Add EOF token
    t_token *eof = create_token(TOKEN_EOF, NULL);
    add_token(tokenizer, eof);
    return true;
}
