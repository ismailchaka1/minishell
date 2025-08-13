/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 21:57:56 by root              #+#    #+#             */
/*   Updated: 2025/08/12 17:42:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../minishell.h"


int check_valid_key(char *str)
{
    int i;

    if (!str || !str[0])
        return 0;

    // First character must be alphabetic or underscore
    if (!(ft_isalpha(str[0]) || str[0] == '_'))
        return 0;

    i = 1;
    while (str[i])
    {
        if (!(ft_isalnum(str[i]) || str[i] == '_'))
            return 0;
        i++;
    }
    return 1;
}

int builtin_export(t_shell *shell, char **args)
{
    int exit_status;
    exit_status = 0;
    int i;

    i = 0;
    
    t_env *current = shell->env;
    if (!args || !*args)
    {
        while (current)
        {
            // printf("asdasdasdsa %s %s\n", current->key, current->value);
            if (current->key && current->value)
            {
                printf("declare -x %s=\"%s\"\n", current->key, current->value);
            }else if (current->key && !current->value)
                printf("declare -x %s\n", current->key);
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
            *equals_sign = '\0';
            char *key = arg;
            char *value = equals_sign + 1;
            // check if key is valid
            if (check_valid_key(key))
            {
                set_env_value(&shell->env, key, value);
            }
            else
            {
                exit_status = 1;
                fprintf(stderr, "export: '%s': not a valid identifier\n", arg);
                continue;
            }
        }
        else
        {
            if (check_valid_key(arg))
                set_env_value(&shell->env, arg, NULL);
            else
                fprintf(stderr, "export: '%s': not a valid identifier\n", arg);
        }
    }
    return exit_status;
}


