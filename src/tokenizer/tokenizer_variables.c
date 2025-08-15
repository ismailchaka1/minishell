/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer_variables.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:45:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 16:45:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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
bool add_split_tokens(t_tokenizer *tokenizer, char *expanded_value)
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
