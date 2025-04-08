/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/26 23:45:08 by ichakank          #+#    #+#             */
/*   Updated: 2025/02/26 23:57:58 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void init_shell()
{
    printf("\033[H\033[J");
    // printf("Welcome to minishell\n");
}
void display_prompt()
{
    printf("minishell> ");
}

int main(void)
{
    char *input;
    init_shell();
    while (1)
    {
        input = readline("minishell > ");
        printf("%s \n", input);
        if (!input)
            break;
        
        free(input);
    }
    return (0);
}