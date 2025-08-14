/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_new.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 23:45:08 by ichakank          #+#    #+#             */
/*   Updated: 2025/08/14 17:43:19 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void handle_input_loop(t_shell *shell);
static int process_input(char *input, t_shell *shell);
static void cleanup_and_exit(t_shell *shell);
static void setup_shell_environment(t_shell *shell, char **envp);

int main(int argc, char **argv, char **envp)
{
    t_shell shell;
    
    (void)argc;
    (void)argv;
    
    setup_shell_environment(&shell, envp);
    handle_input_loop(&shell);
    cleanup_and_exit(&shell);
    
    return 0;
}

static void handle_input_loop(t_shell *shell)
{
    char *input;
    
    while (1)
    {
        input = readline("minishell > ");
        if (!input)
        {
            printf("exit\n"); // Print exit message when Ctrl+D is pressed
            break;
        }
        
        if (*input)
            add_history(input);
            
        if (process_input(input, shell) == -1)
        {
            free(input);
            break;
        }
        
        free(input);
    }
}

static int process_input(char *input, t_shell *shell)
{
    t_tokenizer *tokenizer = init_tokenizer(input);
    
    if (tokenize(tokenizer, shell))
    {
        print_tokens(tokenizer->tokens);
        t_command *commands = parse_tokens(tokenizer->tokens, tokenizer);
        
        if (commands)
        {
            print_commands(commands);
            execute_commands(shell, commands);
            free_commands(commands);
            
            // Check if exit was requested
            if (shell->exit_status == -1)
            {
                free_tokenizer(tokenizer);
                return -1;
            }
        }
    }
    else
    {
        printf("Tokenization failed\n");
    }
    
    free_tokenizer(tokenizer);
    return 0;
}

static void cleanup_and_exit(t_shell *shell)
{
    free_env(shell->env);
}

static void setup_shell_environment(t_shell *shell, char **envp)
{
    clear_screen();
    init_shell(shell, envp);
    setup_interactive_signals();
}
