/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 23:45:08 by ichakank          #+#    #+#             */
/*   Updated: 2025/04/14 12:52:31 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void init_shell()
{
    printf("\033[H\033[J");
    // printf("Welcome to minishell\n");
}

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

int main(void)
{
    char *input;
    init_shell();
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
    return (0);
}