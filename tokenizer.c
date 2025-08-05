/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 15:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/05 15:13:14 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Initialize tokenizer with input string
t_tokenizer *init_tokenizer(char *input)
{
    t_tokenizer *tokenizer = malloc(sizeof(t_tokenizer));
    if (!tokenizer)
        return NULL;
    tokenizer->input = strdup(input); // Copy input
    tokenizer->pos = 0;
    tokenizer->tokens = NULL;
    return tokenizer;
}

// Free tokenizer and its tokens
void free_tokenizer(t_tokenizer *tokenizer)
{
    t_token *current = tokenizer->tokens;
    while (current)
    {
        t_token *next = current->next;
        free(current->value);
        free(current);
        current = next;
    }
    free(tokenizer->input);
    free(tokenizer);
}

// Create a new token
t_token *create_token(t_token_type type, char *value)
{
    t_token *token = malloc(sizeof(t_token));
    if (!token)
        return NULL;
    token->type = type;
    token->value = value ? strdup(value) : NULL;
    token->next = NULL;
    token->prev = NULL; // ← initialize prev
    return token;
}

// Add token to tokenizer's linked list
void add_token(t_tokenizer *tokenizer, t_token *token)
{
    if (!tokenizer->tokens)
    {
        tokenizer->tokens = token;
    }
    else
    {
        t_token *current = tokenizer->tokens;
        while (current->next)
            current = current->next;
        current->next = token;
        token->prev = current;
    }
}

// Check if character is an operator
static bool is_operator(char c)
{
    return (c == '|' || c == '<' || c == '>');
}

// Extract quoted string (single or double quotes) with escape handling
static char *extract_quoted(t_tokenizer *tokenizer, char quote)
{
    tokenizer->pos++; // Skip opening quote
    char *result = ft_strdup("");
    if (!result)
        return NULL;
    
    while (tokenizer->input[tokenizer->pos] && 
           tokenizer->input[tokenizer->pos] != quote)
    {
        char c = tokenizer->input[tokenizer->pos];
        
        // Handle escape sequences
        if (c == '\\' && tokenizer->input[tokenizer->pos + 1])
        {
            char next = tokenizer->input[tokenizer->pos + 1];
            
            // For double quotes, handle common escape sequences
            if (quote == '"')
            {
                if (next == '"' || next == '\\' || next == '$' || next == '`' || next == '\n')
                {
                    // Add the escaped character (except for backslash escaping backslash)
                    char escaped[2] = {(next == '\n') ? '\n' : next, '\0'};
                    char *temp = ft_strjoin(result, escaped);
                    free(result);
                    result = temp;
                    if (!result)
                        return NULL;
                    tokenizer->pos += 2; // Skip backslash and escaped char
                    continue;
                }
                else
                {
                    // Keep the backslash for unrecognized escape sequences
                    char backslash[2] = {'\\', '\0'};
                    char *temp = ft_strjoin(result, backslash);
                    free(result);
                    result = temp;
                    if (!result)
                        return NULL;
                    tokenizer->pos++; // Just skip the backslash, next iteration will handle the character
                    continue;
                }
            }
            // For single quotes in bash, only \' is special when outside quotes
            else if (quote == '\'' && next == '\'')
            {
                // This is actually end of quote + escaped quote + start of new quote
                // We'll handle this in the main tokenization logic
                break;
            }
        }
        
        // Add regular character
        char str[2] = {c, '\0'};
        char *temp = ft_strjoin(result, str);
        free(result);
        result = temp;
        if (!result)
            return NULL;
        tokenizer->pos++;
    }
    
    if (tokenizer->input[tokenizer->pos] != quote)
    {
        printf("Error: Unclosed quote\n");
        free(result);
        return NULL;
    }
    
    tokenizer->pos++; // Skip closing quote
    return result;
}

char *expand_variables(char *str, t_shell *shell)
{
    size_t i = 0;
    char *result = ft_strdup("");
    if (!result)
    {
        // Return a copy of the original string if we can't allocate memory for result
        return ft_strdup(str);
    }

    while (str[i])
    {
        if (str[i] == '$' && str[i + 1] == '?')
        {
            // Handle $? (exit status)
            char *exit_status_str = ft_itoa(shell->exit_status);
            if (!exit_status_str)
                return (free(result), NULL);

            size_t exit_status_len = ft_strlen(exit_status_str);
            char *before = ft_substr(str, 0, i);
            char *after = ft_strdup(str + i + 2); // Skip '$?'
            char *temp1 = ft_strjoin(before, exit_status_str);
            char *new_str = ft_strjoin(temp1, after);

            free(before);
            free(after);
            free(temp1);
            free(exit_status_str);
            free(result);
            result = new_str;
            str = result;
            i = i + exit_status_len; // Move past the inserted exit status
        }
        else if (str[i] == '$' && str[i + 1] == '{')
        {
            // Handle ${VAR} (braced variables)
            size_t j = i + 2;
            while (str[j] && str[j] != '}')
                j++;
            
            if (str[j] == '}')
            {
                char *var_name = ft_substr(str, i + 2, j - (i + 2));
                if (!var_name)
                    return (free(result), NULL);

                char *var_value = get_env_value(shell->env, var_name);
                free(var_name);

                if (!var_value)
                    var_value = "";

                char *before = ft_substr(str, 0, i);
                char *after = ft_strdup(str + j + 1);
                char *temp1 = ft_strjoin(before, var_value);
                char *new_str = ft_strjoin(temp1, after);

                free(before);
                free(after);
                free(temp1);
                free(result);
                result = new_str;
                str = result;
                i = i + ft_strlen(var_value); // Move past the inserted variable
            }
            else
            {
                // Unclosed brace, treat as literal
                i++;
            }
        }
        else if (str[i] == '$' && (ft_isalpha(str[i + 1]) || str[i + 1] == '_'))
        {
            size_t j = i + 1;
            while (ft_isalnum(str[j]) || str[j] == '_')
                j++;

            char *var_name = ft_substr(str, i + 1, j - (i + 1));
            if (!var_name)
                return (free(result), NULL);

            char *var_value = get_env_value(shell->env, var_name);
            free(var_name);

            // Always proceed with expansion, even if undefined (expands to empty)
            if (!var_value)
                var_value = "";

            char *before = ft_substr(str, 0, i);
            char *after = ft_strdup(str + j);
            char *temp1 = ft_strjoin(before, var_value);
            char *new_str = ft_strjoin(temp1, after);

            free(before);
            free(after);
            free(temp1);
            free(result);
            result = new_str;
            if (!result)
                return NULL;
            str = result;
            i = i + ft_strlen(var_value); // Move past the inserted variable
        }
        else if (str[i] == '$' && str[i + 1] == '\0')
        {
            // Literal dollar sign at end of string
            char dollar[2] = {'$', '\0'};
            char *before = ft_substr(str, 0, i);
            char *temp = ft_strjoin(before, dollar);
            free(before);
            free(result);
            result = temp;
            if (!result)
                return NULL;
            i++;
        }
        else if (str[i] == '$' && !ft_isalnum(str[i + 1]) && str[i + 1] != '_' && str[i + 1] != '?' && str[i + 1] != '{')
        {
            // Literal dollar sign followed by non-variable character
            i++;
        }
        else
            i++;
    }
    return result;
}

// Helper function to split a string on whitespace and add multiple tokens
static bool add_split_tokens(t_tokenizer *tokenizer, char *expanded_value)
{
    if (!expanded_value || expanded_value[0] == '\0')
        return true; // Empty string, nothing to add
    
    // Use ft_split to split on spaces
    char **words = ft_split(expanded_value, ' ');
    if (!words)
        return false;
    
    int i = 0;
    while (words[i])
    {
        // Skip empty strings that might result from multiple spaces
        if (words[i][0] != '\0')
        {
            t_token *token = create_token(TOKEN_WORD, words[i]);
            if (!token)
            {
                // Free remaining words and return false
                while (words[i])
                {
                    free(words[i]);
                    i++;
                }
                free(words);
                return false;
            }
            add_token(tokenizer, token);
        }
        free(words[i]);
        i++;
    }
    free(words);
    return true;
}

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

void print_tokens(t_token *tokens)
{
    while (tokens)
    {
        printf("Type: %d, Value: %s|\n", tokens->type, 
               tokens->value ? tokens->value : "NULL");
        tokens = tokens->next;
    }
}
