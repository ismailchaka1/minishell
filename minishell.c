/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 23:45:08 by ichakank          #+#    #+#             */
/*   Updated: 2025/06/17 23:19:23 by root             ###   ########.fr       */
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

// Extract a word (handles variables and unquoted strings)
static char *extract_word(t_tokenizer *tokenizer)
{
    size_t start = tokenizer->pos;
    while (tokenizer->input[tokenizer->pos] && 
           !isspace(tokenizer->input[tokenizer->pos]) &&
           !is_operator(tokenizer->input[tokenizer->pos]) &&
           tokenizer->input[tokenizer->pos] != '\'' &&
           tokenizer->input[tokenizer->pos] != '"')
        tokenizer->pos++;
    size_t len = tokenizer->pos - start;
    char *word = strndup(tokenizer->input + start, len);
    return word;
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

// Main tokenization function
bool tokenize(t_tokenizer *tokenizer)
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
            if (!value)
                return false;
            token = create_token(TOKEN_DOUBLE_QUOTE, value);
            free(value);
        }
        // Handle words
        else if (!isspace(c))
        {
            char *value = extract_word(tokenizer);
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

int main(int argc, char **argv, char **envp)
{
    char *input;
    (void)argc;
    (void)argv; // Unused parameters, but required for main signature
    t_shell shell;
    
    printf("\033[H\033[J"); // Clear screen
    init_shell(&shell, envp);
    // builtin_env(&shell); // Rebuild environment if needed
    builtin_cd(&shell, ".."); // Change to root directory
    builtin_pwd(&shell); // Rebuild environment if needed
    builtin_env(&shell); // Print environment variables
    while (1)
    {
        input = readline("minishell > ");
        if (!input)
            break;
        if (*input)
            add_history(input);
        t_tokenizer *tokenizer = init_tokenizer(input);
        if (tokenize(tokenizer))
            print_tokens(tokenizer->tokens);
        else
            printf("Tokenization failed\n");
        free(input);
    }
    free_env(shell.env);
    return (0);
}