/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 23:45:08 by ichakank          #+#    #+#             */
/*   Updated: 2025/08/03 11:17:51 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <fcntl.h>
#include <sys/stat.h>

t_env *init_env(t_env *env, char **envp)
{
    t_env *head = NULL;
    t_env *current = NULL;
    int count = 0;

    (void)env; // Unused parameter
    if (!envp)
        return NULL;
    
    while (envp[count])
    {
        int i = 0;
        // Find the '=' character
        while (envp[count][i] && envp[count][i] != '=')
            i++;
        
        if (envp[count][i] == '=')
        {
            t_env *new_env = malloc(sizeof(t_env));
            if (!new_env)
                return head;
            new_env->key = strndup(envp[count], i);
            new_env->value = strdup(envp[count] + i + 1);
            new_env->next = NULL;
            
            if (!head)
            {
                head = new_env;
                current = head;
            }
            else
            {
                current->next = new_env;
                current = new_env;
            }
        }
        count++;
    }
    return head;
}

void free_env(t_env *env)
{
    while (env)
    {
        t_env *next = env->next;
        free(env->key);
        free(env->value);
        free(env);
        env = next;
    }
}

char *get_env_value(t_env *env, const char *key)
{
    while (env)
    {
        if (ft_strncmp(env->key, key, ft_strlen(key)) == 0)
            return env->value;
        env = env->next;
    }
    return NULL; // Key not found
}

void set_env_value(t_env *env, const char *key, const char *value)
{
    t_env *current = env;
    while (current)
    {
        if (ft_strncmp(current->key, key, ft_strlen(key)) == 0)
        {
            if (value)
            {      
                free(current->value);
                current->value = strdup(value);
            }
            return;
        }
        current = current->next;
    }
    
    t_env *new_env = malloc(sizeof(t_env));
    if (!new_env)
        return;
    while (env && env->next)
        env = env->next;
    new_env->key = strdup(key);
    if (value)
        new_env->value = strdup(value);
    else
        new_env->value = NULL;
    new_env->next = NULL;
    if (!env)
        env = new_env; 
    else
        env->next = new_env;
}

void print_env(t_env *env)
{
    if (!env)
        return;

    t_env *current = env;
    while (current)
    {
        if (current->key && current->value)
            printf("%s=%s\n", current->key, current->value);
        else if (current->key)
            printf("%s=\n", current->key);
        current = current->next;
    }
}

void init_shell(t_shell *shell, char **envp)
{
    if (!shell)
        return;
    shell->env = init_env(NULL, envp);
    shell->exit_status = 0;
}
// {
//     printf("\033[H\033[J");
//     // printf("Welcome to minishell\n");
// }

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

t_command *parse_tokens(t_token *tokens)
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
                // Check for empty command
                // if (tokens->value[0] == '\0')
                // {
                //     printf("Error: Empty command\n");
                //     free_commands(head);
                //     return NULL;
                // }
                
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
            printf("  Heredoc: true\n");
        commands = commands->next;
    }
}

// Execute a builtin command with redirected stdin/stdout
int execute_builtin(t_command *command, t_shell *shell)
{
    int result = 0;
    int saved_stdout = dup(STDOUT_FILENO);
    int saved_stdin = dup(STDIN_FILENO);

    if (saved_stdout == -1 || saved_stdin == -1)
    {
        perror("dup failed");
        if (saved_stdout != -1)
            close(saved_stdout);
        if (saved_stdin != -1)
            close(saved_stdin);
        return -1;
    }

    // Handle redirections
    if (handle_redirections(command) == -1)
    {
        close(saved_stdout);
        close(saved_stdin);
        return -1;
    }

    if (strcmp(command->command, "cd") == 0)
    {
        result = builtin_cd(shell, command->args);
    }
    else if (strcmp(command->command, "env") == 0)
    {
        result = builtin_env(shell);
    }
    else if (strcmp(command->command, "pwd") == 0)
    {
        result = builtin_pwd(shell);
    }
    else if (strcmp(command->command, "export") == 0)
    {
        result = builtin_export(shell, command->args);
    }
    else if (strcmp(command->command, "unset") == 0)
    {
        result = ft_unset(shell, command->args);
    }
    else if (strcmp(command->command, "exit") == 0)
    {
        shell->exit_status = -1;
        result = 0;
    }

    // Restore original file descriptors
    dup2(saved_stdout, STDOUT_FILENO);
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdout);
    close(saved_stdin);

    return result;
}


int is_builtin_command(const char *command)
{
    return (strcmp(command, "cd") == 0 || 
            strcmp(command, "env") == 0 || 
            strcmp(command, "pwd") == 0 ||
            strcmp(command, "export") == 0 ||
            strcmp(command, "unset") == 0 ||
            strcmp(command, "exit") == 0);
}

void execute_commands(t_shell *shell, t_command *command)
{
    if (!command->command)
    {
        if (handle_standalone_redirections(command, shell) != 0)
        {
            shell->exit_status = 1;
        }
        else
        {
            shell->exit_status = 0;
        }
        command = command->next;
    }

    // Check if this is a builtin command
    if (is_builtin_command(command->command))
    {
        int result = execute_builtin(command, shell);
        shell->exit_status = result;
    }
    else
    {
        execute_external_command(command, shell);
    }
}

// void execute_commands(t_shell *shell, t_command *commands)
// {
//     t_command *current = commands;

//     while (current)
//     {
//         // Handle standalone redirections (no command)
//         if (!current->command)
//         {
//             if (handle_standalone_redirections(current, shell) != 0)
//             {
//                 shell->exit_status = 1;
//             }
//             else
//             {
//                 shell->exit_status = 0;
//             }
//             current = current->next;
//             continue;
//         }
        
//         // Validate command before execution
//         if (current->command[0] == '\0')
//         {
//             printf("Error: : command not found\n");
//             shell->exit_status = 1;
//             current = current->next;
//             continue;
//         }
        
//         // Check if this is part of a pipeline
//         if (current->next)
//         {
//             // This is part of a pipeline - execute the entire pipeline
//             execute_external_command(current, shell);
            
//             // Skip to the end of this pipeline
//             while (current && current->next)
//                 current = current->next;
            
//             // Move to the command after the pipeline
//             current = current->next;
//             continue;
//         }
        
//         // Handle single commands (not part of a pipeline)
//         if (is_builtin_command(current->command))
//         {
//             // Execute builtin command with proper redirection handling
//             int result = execute_builtin(current, shell);
//             shell->exit_status = result;
//         }
//         else
//         {
//             // Execute external command
//             execute_external_command(current, shell);
//         }
        
//         current = current->next;
//     }
// }

void free_commands(t_command *commands)
{
    if (!commands)
        return;
        
    while (commands)
    {
        t_command *next = commands->next;
        
        // Free command string
        if (commands->command)
        {
            free(commands->command);
        }
        
        // Free arguments array
        if (commands->args)
        {
            int i = 0;
            while (commands->args && commands->args[i])
            {
                if (commands->args[i])
                    free(commands->args[i]);
                i++;
            }
            free(commands->args);
        }
        
        // Free file names (only if they exist)
        if (commands->input_file)
            free(commands->input_file);
        if (commands->output_file)
            free(commands->output_file);
        if (commands->heredoc_delimiter)
            free(commands->heredoc_delimiter);
        
        // Free redirections
        if (commands->redirects)
            free_redirects(commands->redirects);
        
        // Free the command structure itself
        free(commands);
        commands = next;
    }
}

// Create a new redirection
t_redirect *create_redirect(char *filename, int type)
{
    return create_redirect_with_quotes(filename, type, false);
}

// Create a new redirection with quote information
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

// Add redirection to command
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

// Free redirections
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

// Handle standalone redirections without commands
int handle_standalone_redirections(t_command *command, t_shell *shell)
{
    int result = 0;
    t_redirect *redirect = command->redirects;
    
    while (redirect)
    {
        switch (redirect->type)
        {
            case 0: // Input redirection
                // printf("Input redirection: < %s\n", redirect->filename);
                if (handle_input_redirection(redirect->filename) != 0)
                    result = 1;
                break;
            case 1: // Output redirection
                // printf("Output redirection: > %s\n", redirect->filename);
                if (handle_output_redirection(redirect->filename, false) != 0)
                    result = 1;
                break;
            case 2: // Append redirection
                // printf("Append redirection: >> %s\n", redirect->filename);
                if (handle_output_redirection(redirect->filename, true) != 0)
                    result = 1;
                break;
            case 3: // Heredoc (parsing only - execution handled by colleague)
                // printf("Heredoc redirection: << %s\n", redirect->filename);
                if (handle_heredoc(redirect->filename, !redirect->quoted_delimiter, shell) != 0)
                    result = 1;
                break;
        }
        redirect = redirect->next;
    }
   
    return result;
}

// Handle output redirection (> or >>)
int handle_output_redirection(char *filename, bool append)
{
    // Check for empty filename
    if (!filename || filename[0] == '\0')
    {
        printf("Error: Empty filename for output redirection\n");
        return 1;
    }
    
    // Check for special directory names
    if (strcmp(filename, ".") == 0)
    {
        printf("Error: %s: Is a directory\n", filename);
        return 1;
    }
    
    if (strcmp(filename, "..") == 0)
    {
        printf("Error: %s: Is a directory\n", filename);
        return 1;
    }
    
    // Check if filename is a directory
    struct stat file_stat;
    if (stat(filename, &file_stat) == 0 && S_ISDIR(file_stat.st_mode))
    {
        printf("Error: %s: Is a directory\n", filename);
        return 1;
    }
    
    int flags = O_WRONLY | O_CREAT;
    if (append)
        flags |= O_APPEND;
    else
        flags |= O_TRUNC;
    
    int fd = open(filename, flags, 0644);
    if (fd == -1)
    {
        perror(filename);
        return 1;
    }
    
    if (append)
        printf("File '%s' opened for appending (or created if it didn't exist)\n", filename);
    else
        printf("File '%s' created/truncated successfully\n", filename);
    
    close(fd);
    return 0;
}

// Handle input redirection (<)
int handle_input_redirection(char *filename)
{
    // Check for empty filename
    if (!filename || filename[0] == '\0')
    {
        printf("Error: Empty filename for input redirection\n");
        return 1;
    }
    
    // Check if filename is a directory
    struct stat file_stat;
    if (stat(filename, &file_stat) == 0 && S_ISDIR(file_stat.st_mode))
    {
        printf("Error: %s: Is a directory\n", filename);
        return 1;
    }
    
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror(filename);
        return 1;
    }
    
    printf("File '%s' opened for reading successfully\n", filename);
    close(fd);
    return 0;
}

// Handle heredoc (<<)
int handle_heredoc(char *delimiter, bool expand_vars, t_shell *shell)
{
    (void)shell;
    (void) expand_vars;
    int fd;
    char *input;
    fd = open(".heredoc", O_WRONLY | O_CREAT | O_TRUNC);
    if (fd != -1)
    {
        while (1)
        {
            input = readline("heredoc > ");
            if (*input)
            {
                if (ft_strncmp(input, delimiter, INT_MAX) == 0)
                {
                    free(input);
                    break;
                }
                write(fd, input, ft_strlen(input));
            }
        }
    }
    unlink(".heredoc");
    return 0;
}

int main(int argc, char **argv, char **envp)
{
    char *input;
    (void)argc;
    (void)argv; // Unused parameters, but required for main signature
    t_shell shell;
    
    printf("\033[H\033[J"); // Clear screen
    init_shell(&shell, envp);
    setup_interactive_signals(); // Setup signal handlers for interactive mode
    // builtin_env(&shell); // Rebuild environment if needed
    // builtin_cd(&shell, ".."); // Change to root directory
    // builtin_pwd(&shell); // Rebuild environment if needed
    // builtin_env(&shell); // Print environment variables
    while (1)
    {
        input = readline("minishell > ");
        if (!input)
        {
            //printf("exit\n"); // Print exit message when Ctrl+D is pressed
            break;
        }
        if (*input)
            add_history(input);
        t_tokenizer *tokenizer = init_tokenizer(input);
        if (tokenize(tokenizer, &shell))
        {
            // print_tokens(tokenizer->tokens);
            // change the tokens linked list to a command linked list
            t_command *commands = parse_tokens(tokenizer->tokens);
            if (commands)
            {
                print_commands(commands);
                execute_commands(&shell, commands);
                free_commands(commands);
                // Check if exit was requested
                if (shell.exit_status == -1)
                {
                    free_tokenizer(tokenizer);
                    free(input);
                    break;
                }
            }
        }
        else
        {
            printf("Tokenization failed\n");
        }
        free_tokenizer(tokenizer);
        free(input); // This is now only called when input is not NULL
    }
    free_env(shell.env);
    return (0);
}
