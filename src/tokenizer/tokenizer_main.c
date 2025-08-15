/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer_main.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:45:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 16:56:15 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Handle pipe and redirection operators
static t_token *handle_operators(t_tokenizer *tokenizer)
{
    char c = tokenizer->input[tokenizer->pos];
    t_token *token = NULL;

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
    return (token);
}

static bool check_heredoc_context(t_tokenizer *tokenizer)
{
    bool after_heredoc = false;
    
    if (tokenizer->tokens)
    {
        t_token *last = tokenizer->tokens;
        while (last->next)
            last = last->next;
        after_heredoc = (last->type == TOKEN_HEREDOC);
    }
    return (after_heredoc);
}

// Handle quoted delimiters after heredoc
static t_token *handle_quoted_delimiter(t_tokenizer *tokenizer, char c)
{
    char *value = extract_quoted(tokenizer, c);
    t_token *token = NULL;
    
    if (!value)
        return (NULL);
    token = create_token((c == '\'') ? TOKEN_SINGLE_QUOTE : TOKEN_DOUBLE_QUOTE, value);
    free(value);
    return (token);
}

// Check if current token is a standalone variable
static bool check_standalone_variable(t_tokenizer *tokenizer, bool after_heredoc)
{
    char c = tokenizer->input[tokenizer->pos];
    size_t temp_pos;
    
    if (c != '$' || after_heredoc)
        return (false);
    
    temp_pos = tokenizer->pos + 1;
    if (tokenizer->input[temp_pos] == '{')
    {
        temp_pos++;
        while (tokenizer->input[temp_pos] && tokenizer->input[temp_pos] != '}')
            temp_pos++;
        if (tokenizer->input[temp_pos] == '}')
            temp_pos++;
    }
    else if (ft_isalpha(tokenizer->input[temp_pos]) || tokenizer->input[temp_pos] == '_')
    {
        while (ft_isalnum(tokenizer->input[temp_pos]) || tokenizer->input[temp_pos] == '_')
            temp_pos++;
    }
    
    return (isspace(tokenizer->input[temp_pos]) || 
            is_operator(tokenizer->input[temp_pos]) || 
            tokenizer->input[temp_pos] == '\0');
}

// Handle standalone variables with word splitting
static t_token *handle_standalone_variable(t_tokenizer *tokenizer, t_shell *shell)
{
    size_t start = tokenizer->pos;
    char *var_str;
    char *expanded;
    t_token *token = NULL;
    
    tokenizer->pos++; // Skip $
    if (tokenizer->input[tokenizer->pos] == '{')
    {
        tokenizer->pos++;
        while (tokenizer->input[tokenizer->pos] && tokenizer->input[tokenizer->pos] != '}')
            tokenizer->pos++;
        if (tokenizer->input[tokenizer->pos] == '}')
            tokenizer->pos++;
    }
    else
    {
        while (ft_isalnum(tokenizer->input[tokenizer->pos]) || tokenizer->input[tokenizer->pos] == '_')
            tokenizer->pos++;
    }
    
    var_str = ft_substr(tokenizer->input, start, tokenizer->pos - start);
    if (!var_str)
        return (NULL);
    
    expanded = expand_variables(var_str, shell);
    free(var_str);
    if (expanded)
    {
        if (!add_split_tokens(tokenizer, expanded))
        {
            free(expanded);
            return (NULL);
        }
        free(expanded);
        return ((t_token *)1); // Special return value to indicate success with splits
    }
    token = create_token(TOKEN_WORD, "");
    return (token);
}

// Handle single quoted part
static char *handle_single_quote_part(t_tokenizer *tokenizer)
{
    return (extract_quoted(tokenizer, '\''));
}

// Handle double quoted part with variable expansion
static char *handle_double_quote_part(t_tokenizer *tokenizer, t_shell *shell)
{
    char *part = extract_quoted(tokenizer, '"');
    char *expanded;
    
    if (!part)
        return (NULL);
    if (ft_strchr(part, '$'))
    {
        expanded = expand_variables(part, shell);
        if (expanded)
        {
            free(part);
            return (expanded);
        }
    }
    return (part);
}

// Handle escaped single quote
static char *handle_escaped_quote(t_tokenizer *tokenizer)
{
    tokenizer->pos += 2; // Skip \'
    return (ft_strdup("'"));
}

// Handle unquoted part with variable expansion
static char *handle_unquoted_part(t_tokenizer *tokenizer, t_shell *shell, bool after_heredoc)
{
    size_t start = tokenizer->pos;
    size_t len;
    char *part;
    char *expanded;
    
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
    
    if (tokenizer->pos <= start)
        return (NULL);
    
    len = tokenizer->pos - start;
    part = ft_substr(tokenizer->input, start, len);
    if (!part)
        return (NULL);
    
    if (ft_strchr(part, '$') && !after_heredoc)
    {
        expanded = expand_variables(part, shell);
        if (expanded)
        {
            free(part);
            return (expanded);
        }
    }
    return (part);
}

// Handle concatenated tokens (quotes + words)
static t_token *handle_concatenated_token(t_tokenizer *tokenizer, t_shell *shell, bool after_heredoc)
{
    char *concatenated = ft_strdup("");
    char *part = NULL;
    char *temp;
    
    if (!concatenated)
        return (NULL);
    
    while (tokenizer->input[tokenizer->pos] && 
           !isspace(tokenizer->input[tokenizer->pos]) && 
           !is_operator(tokenizer->input[tokenizer->pos]))
    {
        char current_char = tokenizer->input[tokenizer->pos];
        
        if (current_char == '\'')
            part = handle_single_quote_part(tokenizer);
        else if (current_char == '"')
            part = handle_double_quote_part(tokenizer, shell);
        else if (current_char == '\\' && tokenizer->input[tokenizer->pos + 1] == '\'')
            part = handle_escaped_quote(tokenizer);
        else
            part = handle_unquoted_part(tokenizer, shell, after_heredoc);
        
        if (!part)
        {
            free(concatenated);
            return (NULL);
        }
        
        temp = ft_strjoin(concatenated, part);
        free(concatenated);
        free(part);
        concatenated = temp;
        if (!concatenated)
            return (NULL);
    }
    
    return (create_token(TOKEN_WORD, concatenated ? concatenated : ""));
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
        if (c == '|' || c == '<' || c == '>')
        {
            token = handle_operators(tokenizer);
        }
        // Handle quotes and words (with concatenation)
        else if (c == '\'' || c == '"' || !isspace(c))
        {
            bool after_heredoc = check_heredoc_context(tokenizer);
            
            // If we're after a heredoc and the current character is a quote,
            // treat it as a quoted delimiter (don't concatenate)
            if (after_heredoc && (c == '\'' || c == '"'))
            {
                token = handle_quoted_delimiter(tokenizer, c);
            }
            else if (check_standalone_variable(tokenizer, after_heredoc))
            {
                token = handle_standalone_variable(tokenizer, shell);
                if (token == (t_token *)1) // Special return value for split tokens
                    continue;
            }
            else
            {
                token = handle_concatenated_token(tokenizer, shell, after_heredoc);
            }
        }

        if (token)
            add_token(tokenizer, token);
        else
        {
            printf("Error: Invalid token at position %zu\n", tokenizer->pos);
            return (false);
        }
    }

    // Add EOF token
    t_token *eof = create_token(TOKEN_EOF, NULL);
    add_token(tokenizer, eof);
    return (true);
}
