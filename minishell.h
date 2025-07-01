/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 23:44:59 by ichakank          #+#    #+#             */
/*   Updated: 2025/06/30 18:51:08 by ichakank         ###   ########.fr       */
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
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "./libft/libft.h"


typedef struct s_env
{
    char *key;          // Environment variable key
    char *value;        // Environment variable value
    struct s_env *next; // Pointer to next environment variable
} t_env;

// Shell data structure
typedef struct s_shell
{
    t_env *env;          // Environment variables
    int exit_status;     // Last command exit status
} t_shell;

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
    struct s_token *next; // Linked list 
    struct s_token *prev; // Pointer to previous token (for bidirectional traversal)
} t_token;

typedef struct s_tokenizer
{
    char *input;         // Input string
    size_t pos;          // Current position in input
    t_token *tokens;     // Linked list of tokens
} t_tokenizer;

// generate me linked list of command that will be executed in execve and hande me redirection and pipes and heredocs
typedef struct s_redirect
{
    char *filename;         // Redirection filename
    int type;              // 0 = input, 1 = output, 2 = append, 3 = heredoc
    bool quoted_delimiter; // For heredoc: true if delimiter was quoted
    struct s_redirect *next;
} t_redirect;

typedef struct s_command
{
    char *command;          // Command to execute
    char **args;            // Arguments for the command
    t_redirect *redirects;  // List of all redirections
    char *input_file;       // Input redirection file (for backward compatibility)
    char *output_file;      // Output redirection file (for backward compatibility)
    bool append;            // Append mode for output redirection (for backward compatibility)
    bool heredoc;           // Indicates if this command uses heredoc (for backward compatibility)
    char *heredoc_delimiter; // Delimiter for heredoc
    bool heredoc_expand;
    char *path;            // Full path to the command (if found in PATH)
    struct s_command *next; // Pointer to the next command in the pipeline
} t_command;

t_tokenizer *init_tokenizer(char *input);
void free_tokenizer(t_tokenizer *tokenizer);
bool tokenize(t_tokenizer *tokenizer, t_shell *shell);
t_token *create_token(t_token_type type, char *value);
void add_token(t_tokenizer *tokenizer, t_token *token);

// Command parsing and management functions
t_command *parse_tokens(t_token *tokens);
void free_commands(t_command *commands);

// Environment management functions
t_env *init_env(t_env *env, char **envp);
void free_env(t_env *env);
void print_env(t_env *env);
void init_shell(t_shell *shell, char **envp);

// Built-in command functions
int builtin_env(t_shell *shell);
char *get_env_value(t_env *env, const char *key);
int builtin_pwd(t_shell *shell);
int builtin_cd(t_shell *shell, char **path);
int updateOLDPWD(t_shell *shell, char *path);

// Signal handling functions
void setup_interactive_signals(void);
void setup_execution_signals(void);
void setup_heredoc_signals(void);

// Redirection management functions
t_redirect *create_redirect(char *filename, int type);
t_redirect *create_redirect_with_quotes(char *filename, int type, bool quoted);
void add_redirect(t_command *command, t_redirect *redirect);
void free_redirects(t_redirect *redirects);
int handle_redirections(t_command *command);

// Standalone redirection functions
int handle_standalone_redirections(t_command *command, t_shell *shell);
int handle_output_redirection(char *filename, bool append);
int handle_input_redirection(char *filename);
int handle_heredoc(char *delimiter, bool expand_vars, t_shell *shell);


// Start of execution
void execute_external_command(t_command *commands, t_shell *shell);
void execute_single_command(t_command *command, t_shell *shell, int input_fd, int output_fd);
void execute_pipeline(t_command *commands, t_shell *shell);
char **create_args_array(t_command *command);
int execute_builtin(t_command *command, t_shell *shell);
int is_builtin_command(const char *command);

#endif // MINISHELL_H
