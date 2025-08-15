/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer_quotes.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:45:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 16:45:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Check if character is an operator
bool is_operator(char c)
{
    return (c == '|' || c == '<' || c == '>');
}

// Extract quoted string (single or double quotes) with escape handling
char *extract_quoted(t_tokenizer *tokenizer, char quote)
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
