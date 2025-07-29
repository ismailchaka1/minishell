/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 21:57:56 by root              #+#    #+#             */
/*   Updated: 2025/07/29 22:35:30 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../minishell.h"

int builtin_export(t_shell *shell, char **args)
{
    int exit_status;
    exit_status = 0;
    int i;

    i = 0;
    
    t_env *current = shell->env;
    if (!args || !*args)
    {
        while (current && current->next)
        {
            if (current->key && current->value)
            {
                printf("declare -x %s=\"%s\"\n", current->key, current->value);
            }
            current = current->next;
        }
        return 0;
    }
    while (args[i])
    {
        if (args[i][0] == '\0')
        {
            fprintf(stderr, "export: '%s': not a valid identifier\n", args[i]);
            exit_status = 1;
            i++;
            continue;
        }
        i++;
    }
    for (int i = 0; args[i]; i++)
    {
        char *arg = args[i];
        char *equals_sign = strchr(arg, '=');

        if (equals_sign)
        {
            // Split into key and value
            *equals_sign = '\0';
            char *key = arg;
            char *value = equals_sign + 1;
            // check if key is valid
            if (ft_isalpha(key[0]) || key[0] == '_')
            {
                // Set the environment variable
                set_env_value(shell->env, key, value);
            }
            else
            {
                exit_status = 1;
                fprintf(stderr, "export: '%s': not a valid identifier\n", key);
                continue;
            }
        }
    }
    return exit_status;
}