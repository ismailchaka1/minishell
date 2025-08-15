/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/14 00:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/14 00:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Forward declaration
static void print_redirections_debug(t_redirect *redirects);

void print_tokens(t_token *tokens)
{
    while (tokens)
    {
        printf("Type: %d, Value: %s|\n", tokens->type, 
               tokens->value ? tokens->value : "NULL");
        tokens = tokens->next;
    }
}

void print_commands_debug(t_command *commands)
{
    t_command *current = commands;
    int cmd_num = 1;
    
    while (current)
    {
        printf("Command %d:\n", cmd_num);
        printf("  Command: %s\n", current->command ? current->command : "NULL");
        
        if (current->args)
        {
            printf("  Args: ");
            for (int i = 0; current->args[i]; i++)
                printf("'%s' ", current->args[i]);
            printf("\n");
        }
        
        if (current->redirects)
            print_redirections_debug(current->redirects);
            
        current = current->next;
        cmd_num++;
    }
}

void print_redirections_debug(t_redirect *redirects)
{
    t_redirect *current = redirects;
    
    while (current)
    {
        printf("  Redirection: ");
        switch (current->type)
        {
            case 0:
                printf("< %s\n", current->filename);
                break;
            case 1:
                printf("> %s\n", current->filename);
                break;
            case 2:
                printf(">> %s\n", current->filename);
                break;
            case 3:
                printf("<< %s\n", current->filename);
                break;
        }
        current = current->next;
    }
}

void clear_screen(void)
{
    printf("\033[H\033[J");
}
