/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:30:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 16:15:25 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void print_commands(t_command *commands)
{
    while (commands)
    {
        printf("Command: %s\n", commands->command);
        if (commands->args)
        {
            printf("  Args: ");
            for (int i = 0; commands->args[i]; i++)
                printf("%d %s|\n", i, commands->args[i]);
            printf("\n");
        }
        
        // Print all redirections
        if (commands->redirects)
        {
            printf("  Redirections:\n");
            t_redirect *redirect = commands->redirects;
            while (redirect)
            {
                printf("    %s -> %s\n", 
                    (redirect->type == 0) ? "Input" :
                    (redirect->type == 1) ? "Output" :
                    (redirect->type == 2) ? "Append" : "Heredoc",
                    redirect->filename);
                redirect = redirect->next;
            }
        }
        
        // Keep backward compatibility output
        if (commands->input_file)
            printf("  Input file: %s\n", commands->input_file);
        if (commands->output_file)
            printf("  Output file: %s\n", commands->output_file);
        if (commands->append)
            printf("  Append mode: true\n");
        if (commands->heredoc)
            printf("  Heredoc: %s\n", commands->heredoc_delimiter);
        commands = commands->next;
    }
}
