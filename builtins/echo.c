/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   echo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 17:51:45 by root              #+#    #+#             */
/*   Updated: 2025/08/13 00:17:52 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int builtin_echo(t_shell *shell, char **args)
{
    int i = 1;
    int newline = 1;
    if (!args || !*args)
    {
        printf("\n");
        return 0;
    }

    // Check for -n option
    if (args[1] && strcmp(args[1], "-n") == 0)
    {
        newline = 0;
    }

    while (args[i])
    {
        printf("%s", args[i]);
        if (args[i + 1])
            printf(" ");
        i++;
    }

    if (newline)
        printf("\n");

    return 0;
}
