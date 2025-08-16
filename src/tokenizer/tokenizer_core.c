/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer_core.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:45:00 by root              #+#    #+#             */
/*   Updated: 2025/08/16 13:28:47 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_tokenizer	*init_tokenizer(char *input, t_shell *shell)
{
	t_tokenizer	*tokenizer;

	tokenizer = malloc(sizeof(t_tokenizer));
	if (!tokenizer)
		return (NULL);
	tokenizer->input = strdup(input);
	tokenizer->pos = 0;
	tokenizer->tokens = NULL;
	tokenizer->shell = shell;
	return (tokenizer);
}

void	free_tokenizer(t_tokenizer *tokenizer)
{
	t_token	*current;
	t_token	*next;

	current = tokenizer->tokens;
	while (current)
	{
		next = current->next;
		free(current->value);
		free(current);
		current = next;
	}
	free(tokenizer->input);
	free(tokenizer);
}

t_token	*create_token(t_token_type type, char *value)
{
	t_token	*token;

	token = malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->type = type;
	if (value)
		token->value = strdup(value);
	else
		token->value = NULL;
	token->next = NULL;
	token->prev = NULL;
	return (token);
}

void	add_token(t_tokenizer *tokenizer, t_token *token)
{
	t_token	*current;

	if (!tokenizer->tokens)
	{
		tokenizer->tokens = token;
	}
	else
	{
		current = tokenizer->tokens;
		while (current->next)
			current = current->next;
		current->next = token;
		token->prev = current;
	}
}
