/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 23:45:08 by ichakank          #+#    #+#             */
/*   Updated: 2025/08/05 21:16:15 by root             ###   ########.fr       */
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

// Execute a builtin command with redirected stdin/stdout
int execute_builtin(t_command *command, t_shell *shell, bool pipe)
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
    if (!pipe)
    {
        if (handle_redirections(command) == -1)
        {
            close(saved_stdout);
            close(saved_stdin);
            return -1;
        }
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

    // // Check if this is a builtin command
    // if (is_builtin_command(command->command))
    // {
    //     int result = execute_builtin(command, shell);
    //     shell->exit_status = result;
    // }
    // else
    // {
        execute_external_command(command, shell);
    // }
}
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
    
    // Setup heredoc signal handling
    setup_heredoc_signals();
    g_heredoc_interrupted = 0;
    
    fd = open(".heredoc", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("heredoc file creation");
        setup_interactive_signals();
        return -1;
    }
    
    while (1)
    {
        // Check for interruption before readline
        if (g_heredoc_interrupted)
            break;
            
        input = readline("heredoc > ");
        
        // Check for EOF (Ctrl-D)
        if (!input)
        {
            printf("\n");
            break;
        }
        
        // Check for interruption after readline
        if (g_heredoc_interrupted)
        {
            free(input);
            break;
        }
        
        if (*input)
        {
            size_t input_len = ft_strlen(input);
            size_t delimiter_len = ft_strlen(delimiter);
            
            if (input_len == delimiter_len && 
                ft_strncmp(input, delimiter, delimiter_len) == 0)
            {
                free(input);
                break;
            }
            write(fd, input, input_len);
            write(fd, "\n", 1);
        }
        free(input);
    }
    
    close(fd);
    unlink(".heredoc");
    setup_interactive_signals(); // Restore signals
    
    // Return -1 if interrupted, 0 if completed normally
    return (g_heredoc_interrupted ? -1 : 0);
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
            t_command *commands = parse_tokens(tokenizer->tokens, tokenizer);
            if (commands)
            {
                // print_commands(commands);
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
