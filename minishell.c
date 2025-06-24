/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 23:45:08 by ichakank          #+#    #+#             */
/*   Updated: 2025/06/24 00:24:42 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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
    return token;
}

// Add token to tokenizer's linked list
void add_token(t_tokenizer *tokenizer, t_token *token)
{
    if (!tokenizer->tokens)
        tokenizer->tokens = token;
    else
    {
        t_token *current = tokenizer->tokens;
        while (current->next)
            current = current->next;
        current->next = token;
    }
}

// Check if character is an operator
static bool is_operator(char c)
{
    return (c == '|' || c == '<' || c == '>');
}

// Extract quoted string (single or double quotes)
static char *extract_quoted(t_tokenizer *tokenizer, char quote)
{
    size_t start = tokenizer->pos + 1; // Skip opening quote
    tokenizer->pos++;
    while (tokenizer->input[tokenizer->pos] && 
           tokenizer->input[tokenizer->pos] != quote)
        tokenizer->pos++;
    if (tokenizer->input[tokenizer->pos] != quote)
    {
        printf("Error: Unclosed quote\n");
        return NULL;
    }
    size_t len = tokenizer->pos - start;
    char *str = strndup(tokenizer->input + start, len);
    tokenizer->pos++; // Skip closing quote
    return str;
}
char *expand_variables(char *str, t_shell *shell)
{
    size_t i = 0;
    size_t len;

    len = ft_strlen(str);
    while (i < len)
    {
        if (str[i] == '$')
        {
            size_t j = i + 1;
            while (j < len && (ft_isalnum(str[j]) || str[j] == '_'))
                j++;
            char *var_name = strndup(str + i + 1, j - i - 1);
            char *var_value = get_env_value(shell->env, var_name);
            free(var_name);
            if (var_value)
            {
                // Replace variable with its value
                size_t var_len = ft_strlen(var_value);
                char *new_str = malloc(len - (j - i) + var_len + 1);
                if (!new_str)
                    return NULL;
                strncpy(new_str, str, i);
                strcpy(new_str + i, var_value);
                strcpy(new_str + i + var_len, str + j);
                return new_str;
            }else
            {
                // Variable not found, just skip it
                char *new_str = malloc(len - (j - i) + 1);
                if (!new_str)
                    return NULL;
                strncpy(new_str, str, i);
                strcpy(new_str + i, str + j);
                return new_str;
            }
        }else{
            i++;
        }
    }
    return NULL;
}

static char *extract_word(t_tokenizer *tokenizer, t_shell *shell)
{
    size_t lenght = 0;
    size_t start = tokenizer->pos;
    char *quoted;
    bool inQuote = false;
    while (isspace(tokenizer->input[tokenizer->pos]))
        tokenizer->pos++;
    while (tokenizer->input[tokenizer->pos])
    {
        char c = tokenizer->input[tokenizer->pos];
        if (!inQuote && (isspace(c) || is_operator(c)))
            break;
        if (c == '\'' || c == '"')
        {
            inQuote = !inQuote; 
            quoted = extract_quoted(tokenizer, c);
            printf("quoted: %s\n", quoted);
            if (!quoted)
                return "hamid";
            lenght += ft_strlen(quoted);
        }else
        {
            tokenizer->pos++;
            lenght++;
        }
    }
    inQuote = 0;
    printf("lenght: %zu\n", lenght);
    char *word = malloc(lenght + 1);
    if (!word)
        return NULL;
    size_t i = 0;
    tokenizer->pos = start;
    while (isspace(tokenizer->input[tokenizer->pos]))
        tokenizer->pos++;
    while (tokenizer->input[tokenizer->pos])
    {
        char c = tokenizer->input[tokenizer->pos];
        if (!inQuote && (isspace(c) || is_operator(c)))
            break;
    
        if (c == '\'' || c == '"')
        {
            inQuote = !inQuote;
            char *quoted = extract_quoted(tokenizer, c);
            if (!quoted)
            {
                free(word);
                return NULL;
            }
            size_t j = 0;
            if (c == '"' && ft_strchr(quoted, '$'))
            {
                printf("Expanding variables in: %s\n", quoted);
                char *expanded = expand_variables(quoted, shell);
                free(quoted);
                quoted = expanded;
            }
            while (quoted[j])
                word[i++] = quoted[j++];
            free(quoted);
        }
        else
        {
            word[i++] = c;
            tokenizer->pos++;
        }
    }
    word[i] = '\0';
    return word;
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
        // Handle quotes
        else if (c == '\'')
        {
            char *value = extract_quoted(tokenizer, '\'');
            if (!value)
                return false;
            token = create_token(TOKEN_SINGLE_QUOTE, value);
            free(value);
        }
        else if (c == '"')
        {
            char *value = extract_quoted(tokenizer, '"');
            char *expanded_value = expand_variables(value, shell);
            if (!value)
                return false;
            token = create_token(TOKEN_DOUBLE_QUOTE, expanded_value);
            free(value);
        }
        // Handle words
        else if (!isspace(c))
        {
            char *value = extract_word(tokenizer, shell);
            token = create_token(TOKEN_WORD, value);
            free(value);
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
        printf("Type: %d, Value: %s\n", tokens->type, 
               tokens->value ? tokens->value : "NULL");
        tokens = tokens->next;
    }
}

t_command *parse_tokens(t_token *tokens)
{
    t_command *head = NULL;
    t_command *current = NULL;

    while (tokens && tokens->type != TOKEN_EOF)
    {
        if (tokens->type == TOKEN_WORD)
        {
            if (!current)
            {
                current = malloc(sizeof(t_command));
                if (!current)
                    return NULL;
                current->command = strdup(tokens->value);
                current->args = NULL;
                current->input_file = NULL;
                current->output_file = NULL;
                current->append = false;
                current->heredoc = false;
                current->next = NULL;

                if (!head)
                    head = current;
                else
                    head->next = current;
            }
            else
            {
                char **new_args = NULL;
                if (current->args)
                {
                    int count = 0;
                    while (current->args[count])
                        count++;
                    new_args = malloc(sizeof(char *) * (count + 2));
                    for (int i = 0; i < count; i++)
                        new_args[i] = current->args[i];
                    new_args[count] = strdup(tokens->value);
                    new_args[count + 1] = NULL;
                    free(current->args);
                }
                else
                {
                    new_args = malloc(sizeof(char *) * 2);
                    new_args[0] = strdup(tokens->value);
                    new_args[1] = NULL;
                }
                current->args = new_args;
            }
        }else if (tokens->type == TOKEN_PIPE)
        {
            if (current)
            {
                if (tokens->next && tokens->next->type == TOKEN_WORD)
                {
                    current->next = malloc(sizeof(t_command));
                    if (!current->next)
                        return NULL;
                    current = current->next;
                    current->command = strdup(tokens->next->value);
                    current->args = NULL;
                    current->input_file = NULL;
                    current->output_file = NULL;
                    current->append = false;
                    current->heredoc = false;
                    current->next = NULL;
                    tokens = tokens->next;
                }
                else
                {
                    printf("Error: Pipe without command\n");
                    return NULL;
                }
            }
            else
            {
                printf("Error: Pipe without command\n");
                return NULL;
            }
        }else if (tokens->type == TOKEN_REDIRECT_IN || 
                  tokens->type == TOKEN_REDIRECT_OUT || 
                  tokens->type == TOKEN_APPEND || 
                  tokens->type == TOKEN_HEREDOC)
        {
            if (!current)
            {
                printf("Error: Redirection without command\n");
                return NULL;
            }
            if (tokens->type == TOKEN_REDIRECT_IN)
                current->input_file = strdup(tokens->next ? tokens->next->value : "");
            else if (tokens->type == TOKEN_REDIRECT_OUT)
                current->output_file = strdup(tokens->next ? tokens->next->value : "");
            else if (tokens->type == TOKEN_APPEND)
            {
                current->output_file = strdup(tokens->next ? tokens->next->value : "");
                current->append = true;
            }
            else if (tokens->type == TOKEN_HEREDOC)
            {
                current->heredoc = true;
                current->input_file = strdup(tokens->next ? tokens->next->value : "");
            }
            if (tokens->next)
                tokens = tokens->next;
        }else if (tokens->type == TOKEN_SINGLE_QUOTE || 
                  tokens->type == TOKEN_DOUBLE_QUOTE)
        {
            char *value = tokens->value ? strdup(tokens->value) : NULL;
            if (!value)
            {
                printf("Error: Quoted string without value\n");
                return NULL;
            }
            if (current)
            {
                current->args = realloc(current->args, sizeof(char *) * 2);
                current->args[0] = value;
                current->args[1] = NULL;
            }
            else
            {
                current = malloc(sizeof(t_command));
                if (!current)
                {
                    free(value);
                    return NULL;
                }
                current->command = value;
                current->args = NULL;
                current->input_file = NULL;
                current->output_file = NULL;
                current->append = false;
                current->heredoc = false;
                current->next = NULL;

                if (!head)
                    head = current;
                else
                    head->next = current;
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
                printf("%s ", commands->args[i]);
            printf("\n");
        }
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

int main(int argc, char **argv, char **envp)
{
    char *input;
    (void)argc;
    (void)argv; // Unused parameters, but required for main signature
    t_shell shell;
    
    printf("\033[H\033[J"); // Clear screen
    init_shell(&shell, envp);
    // builtin_env(&shell); // Rebuild environment if needed
    // builtin_cd(&shell, ".."); // Change to root directory
    // builtin_pwd(&shell); // Rebuild environment if needed
    // builtin_env(&shell); // Print environment variables
    while (1)
    {
        input = readline("minishell > ");
        if (!input)
            break;
        if (*input)
            add_history(input);
        t_tokenizer *tokenizer = init_tokenizer(input);
        if (tokenize(tokenizer, &shell))
        {
            // change the tokens linked list to a command linked list
            t_command *commands = parse_tokens(tokenizer->tokens);
            if (commands)
            {
                print_commands(commands);
                // execute_commands(&shell, commands);
                // free_commands(commands);
            }
            free_tokenizer(tokenizer);
        }
            // print_tokens(tokenizer->tokens);
        else
            printf("Tokenization failed\n");
        free(input);
    }
    free_env(shell.env);
    return (0);
}