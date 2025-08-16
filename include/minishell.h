/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 23:44:59 by ichakank          #+#    #+#             */
/*   Updated: 2025/08/16 13:33:19 by root             ###   ########.fr       */
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
#include "../lib/libft/libft.h"


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
    t_shell *shell;      // Shell instance associated with the tokenizer
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
    t_shell *shell;
    char *path;            // Full path to the command (if found in PATH)
    struct s_tokenizer *tokens; 
    struct s_command *next;
} t_command;

t_tokenizer *init_tokenizer(char *input, t_shell *shell);
void free_tokenizer(t_tokenizer *tokenizer);
bool tokenize(t_tokenizer *tokenizer, t_shell *shell);
t_token *create_token(t_token_type type, char *value);
void add_token(t_tokenizer *tokenizer, t_token *token);

// Tokenizer variable expansion functions
char *expand_variables(char *str, t_shell *shell);
bool add_split_tokens(t_tokenizer *tokenizer, char *expanded_value);

// Tokenizer quote and operator functions
bool is_operator(char c);
char *extract_quoted(t_tokenizer *tokenizer, char quote);

// Command parsing and management functions
t_command *parse_tokens(t_token *tokens, t_tokenizer *tokenizer);
void free_commands(t_command *commands);

// Parsing core functions
t_command *create_new_command(char *command_name, t_tokenizer *tokenizer);
bool add_argument_to_command(t_command *current, char *value);
t_command *handle_pipe_token(t_command *current, t_token *tokens, t_tokenizer *tokenizer);
bool add_arg(t_command *current, char *arg);

// Parsing token handling functions
t_command *handle_word_token(t_command *current, t_command *head, 
                            t_token *tokens, t_tokenizer *tokenizer);
t_command *handle_quoted_token(t_command *current, t_command *head,
                              t_token *tokens, t_tokenizer *tokenizer);

// Parsing redirection functions
t_command *handle_redirection_token(t_command *current, t_command *head,
                                   t_token *tokens, t_tokenizer *tokenizer);

// Environment management functions
t_env *init_env(t_env *env, char **envp);
void free_env(t_env *env);
void print_env(t_env *env);
void init_shell(t_shell *shell, char **envp);

// Built-in command functions
int builtin_env(t_shell *shell);
char *get_env_value(t_env *env, const char *key);
void set_env_value(t_env **env, const char *key, const char *value);
int builtin_pwd(t_shell *shell);
int builtin_cd(t_shell *shell, char **path);
int builtin_export(t_shell *shell, char **args);
int    ft_unset(t_shell *shell, char **args);
int updateOLDPWD(t_shell *shell, char *path);

// Signal handling functions
void setup_interactive_signals(void);
void setup_heredoc_signals(void);

// Global flag for heredoc interruption
extern volatile int g_heredoc_interrupted;

// Redirection management functions
t_redirect *create_redirect(char *filename, int type);
t_redirect *create_redirect_with_quotes(char *filename, int type, bool quoted);
void add_redirect(t_command *command, t_redirect *redirect);
void free_redirects(t_redirect *redirects);
int handle_redirections(t_command *command);

// New redirection helper functions
int handle_command_input_redirection(t_redirect *redirect);
int handle_command_output_redirection(t_redirect *redirect);
int handle_command_heredoc(t_redirect *redirect);
int create_heredoc_file(t_redirect *redirect);
int apply_heredoc_redirection(void);
int handle_output_redirect(t_redirect *redirect);
int handle_append_redirect(t_redirect *redirect);
int setup_heredoc_file(void);
int write_heredoc_line(int fd, char *input);
int check_delimiter_match(char *input, t_redirect *redirect);
int read_heredoc_input(int fd, t_redirect *redirect);
int finalize_heredoc(int fd);
t_redirect *find_last_heredoc(t_command *command);
int process_single_redirect(t_redirect *redirect, t_redirect *last_heredoc);
int preprocess_heredocs(t_command *commands);

// Standalone redirection functions
int handle_standalone_redirections(t_command *command, t_shell *shell);
int handle_output_redirection(char *filename, bool append);
int handle_input_redirection(char *filename);
int handle_heredoc(char *delimiter, bool expand_vars, t_shell *shell);


void execute_external_command(t_command *commands, t_shell *shell);
void execute_single_command(t_command *command, t_shell *shell);
void execute_pipeline(t_command *commands, t_shell *shell);
void execute_pipeline_loop(t_command *commands, t_shell *shell, pid_t *pids, int *prev_pipe_read);
void wait_for_children(pid_t *pids, int cmd_count, t_shell *shell);
void execute_pipeline_child(t_command *current, t_command *commands, int pipefd[2], int prev_pipe_read);
void execute_pipeline_parent(t_command *current, t_shell *shell, pid_t *pids, int *i);
void handle_child_process_pipeline(t_command *current, t_command *commands, char **exec_args, char **env_array);
char **create_args_array(t_command *command);
int execute_builtin(t_command *command, t_shell *shell, bool pipe);
int is_builtin_command(const char *command);
void print_commands(t_command *commands);
void execute_commands(t_shell *shell, t_command *command);

// Execution memory management functions
void free_double_env(char **env_array);
char **build_double_env(t_shell *shell, char **env_array, int count);
char **get_double_env(t_shell *shell);

// Execution path resolution functions
int check_absolute_or_relative(t_command *command);
void check_path(t_command *command, char **paths);
void get_paths(t_command *command, t_shell *shell);
char *build_full_path(char *path, char *command);
void handle_absolute_path(t_command *command, char **paths_orig);

// Execution error handling functions
void handle_command_not_found(t_command *command, t_shell *shell);
void handle_execve_error(t_command *command, char **env_array, char **exec_args, t_shell *shell);
void handle_execve_error_continued(t_command *command, char **env_array, char **exec_args, t_shell *shell);

// Execution process management functions
void free_exec_requirement(t_command *command, char **env_array, char **exec_args);
void free_parsing(t_command *command, t_shell *shell);
void execute_child_process(t_command *command, char **env_array, char **exec_args, t_shell *shell);
int prepare_execution(t_command *command, t_shell *shell, char ***env_array, char ***exec_args);
void handle_parent_process(pid_t pid, t_shell *shell, char **env_array, char **exec_args);

// Debug utility functions (debug_utils.c)
void print_tokens(t_token *tokens);
void print_commands_debug(t_command *commands);
void clear_screen(void);

// Environment manager functions (env_manager.c)
// (Functions already declared above: init_env, free_env, get_env_value, set_env_value, print_env)

// Command manager functions (command_manager.c)
// (Functions already declared above: execute_commands, execute_builtin, is_builtin_command, free_commands, init_shell)

// Redirection utils functions (redirection_utils.c)
// (Functions already declared above: create_redirect, create_redirect_with_quotes, add_redirect, free_redirects, handle_standalone_redirections)

// File operations functions (file_operations.c)
// (Functions already declared above: handle_output_redirection, handle_input_redirection, handle_heredoc)

#endif // MINISHELL_H
