/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 23:45:08 by ichakank          #+#    #+#             */
/*   Updated: 2025/08/14 00:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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
            printf("exit\n"); // Print exit message when Ctrl+D is pressed
            break;
        }
        if (*input)
            add_history(input);
        t_tokenizer *tokenizer = init_tokenizer(input);
        if (tokenize(tokenizer, &shell))
        {
            print_tokens(tokenizer->tokens);
            // change the tokens linked list to a command linked list
            t_command *commands = parse_tokens(tokenizer->tokens, tokenizer);
            if (commands)
            {
                print_commands(commands);
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
