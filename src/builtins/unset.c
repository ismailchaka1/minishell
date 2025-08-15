/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 08:47:14 by ichakank          #+#    #+#             */
/*   Updated: 2025/08/01 09:02:20 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int is_valid_identifier(const char *name)
{
    int i;

    if (!name || !*name)
        return 0;
    
    // Can't start with a digit
    if (ft_isdigit(name[0]))
        return 0;
    
    i = 0;
    while (name[i])
    {
        if (!ft_isalnum(name[i]) && name[i] != '_')
            return 0;
        i++;
    }
    return 1;
}

void ft_unsetenv(char *key, t_shell *shell)
{
    t_env *current = shell->env;
    t_env *prev = NULL;

    if (!key || !shell)
        return;

    while (current)
    {
        if (ft_strlen(current->key) == ft_strlen(key) && 
            ft_strncmp(current->key, key, ft_strlen(key)) == 0)
        {
            if (prev)
                prev->next = current->next;
            else
                shell->env = current->next;
            free(current->key);
            free(current->value);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

int    ft_unset(t_shell *shell, char **args)
{
    int i;

    if (!args || !shell)
        return 0;

    if (!args[0])
        return 0;

    i = 0;
    while (args[i])
    {
        if (!is_valid_identifier(args[i]))
        {
            continue;
        }
        else
        {
            ft_unsetenv(args[i], shell);
        }
        i++;
    }
    return 0;
}