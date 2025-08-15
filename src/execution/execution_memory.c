/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execution_memory.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 17:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/15 17:00:00 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void free_double_env(char **env_array)
{
    char **original = env_array;
    
    if (!env_array)
        return;

    while (*env_array)
    {
        free(*env_array);
        env_array++;
    }
    free(original);
}

char **build_double_env(t_shell *shell, char **env_array, int count)
{
    t_env *current = shell->env;
    int i;

    i = 0;
    if (!env_array || count <= 0)
        return NULL;

    while (current && i < count)
    {
        char *key_equals = ft_strjoin(current->key, "=");
        if (!key_equals)
            return (perror("ft_strjoin"), NULL);
        
        env_array[i] = ft_strjoin(key_equals, current->value);
        free(key_equals);
        
        if (!env_array[i])
            return (perror("ft_strjoin"), NULL);
        current = current->next;
        i++;
    }
    env_array[i] = NULL;
    return env_array;
}

char **get_double_env(t_shell *shell)
{
    t_env *current = shell->env;
    int count;
    
    count = 0;
    while (current)
    {
        count++;
        current = current->next;
    }
    
    char **env_array = malloc(sizeof(char *) * (count + 1));
    if (!env_array)
    {
        perror("malloc");
        return NULL;
    }
    
    current = shell->env;
    env_array = build_double_env(shell, env_array, count);
    return env_array;
}

char **create_args_array(t_command *command)
{
    int arg_count;
    int i;

    i = 0;
    arg_count = 0;
    if (command->args)
    {
        while (command->args[arg_count])
            arg_count++;
    }
    
    char **exec_args = malloc(sizeof(char *) * (arg_count + 2));
    if (!exec_args)
    {
        perror("malloc");
        return NULL;
    }
    exec_args[0] = command->command;
    while (i < arg_count)
    {
        exec_args[i + 1] = command->args[i];
        i++;
    }
    exec_args[arg_count + 1] = NULL;
    return exec_args;
}
