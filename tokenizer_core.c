/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer_core.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:45:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 16:45:00 by root             ###   ########.fr       */
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
