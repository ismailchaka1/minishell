/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 23:44:59 by ichakank          #+#    #+#             */
/*   Updated: 2025/04/14 12:51:35 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
#define MINISHELL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef enum e_token_type
{
    TOKEN_WORD,          // Commands, arguments, or filenames
    TOKEN_PIPE,          // |
    TOKEN_REDIRECT_IN,   // <
    TOKEN_REDIRECT_OUT,  // >
    TOKEN_APPEND,        // >>
    TOKEN_HEREDOC,       // <<
    TOKEN_SINGLE_QUOTE,  // 'string'
    TOKEN_DOUBLE_QUOTE,  // "string"
    TOKEN_EOF            // End of input
} t_token_type;


typedef struct s_token
{
    t_token_type type;   // Type of token
    char *value;         // Token string content
    struct s_token *next; // Linked list pointer
} t_token;

typedef struct s_tokenizer
{
    char *input;         // Input string
    size_t pos;          // Current position in input
    t_token *tokens;     // Linked list of tokens
} t_tokenizer;

t_tokenizer *init_tokenizer(char *input);
void free_tokenizer(t_tokenizer *tokenizer);
bool tokenize(t_tokenizer *tokenizer);
t_token *create_token(t_token_type type, char *value);
void add_token(t_tokenizer *tokenizer, t_token *token);

#endif // MINISHELL_H