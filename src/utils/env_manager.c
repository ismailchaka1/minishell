/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_manager.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/14 00:00:00 by root              #+#    #+#             */
/*   Updated: 2025/08/14 17:43:19 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_env *init_env(t_env *env, char **envp)
{
    t_env *head = NULL;
    t_env *current = NULL;
    int count = 0;

    (void)env; // Unused parameter
    if (!envp)
        return NULL;
    
    while (envp[count])
    {
        int i = 0;
        // Find the '=' character
        while (envp[count][i] && envp[count][i] != '=')
            i++;
        
        if (envp[count][i] == '=')
        {
            t_env *new_env = malloc(sizeof(t_env));
            if (!new_env)
                return head;
            new_env->key = strndup(envp[count], i);
            new_env->value = strdup(envp[count] + i + 1);
            new_env->next = NULL;
            
            if (!head)
            {
                head = new_env;
                current = head;
            }
            else
            {
                current->next = new_env;
                current = new_env;
            }
        }
        count++;
    }
    return head;
}

void free_env(t_env *env)
{
    while (env)
    {
        t_env *next = env->next;
        free(env->key);
        free(env->value);
        free(env);
        env = next;
    }
}

char *get_env_value(t_env *env, const char *key)
{
    while (env)
    {
        if (ft_strncmp(env->key, key, ft_strlen(key)) == 0)
            return env->value;
        env = env->next;
    }
    return NULL; // Key not found
}

void set_env_value(t_env **env, const char *key, const char *value)
{
    t_env *current = *env;

    if (!*env)
    {
        t_env *new_env = malloc(sizeof(t_env));
        if (!new_env)
            return;
        new_env->key = strdup(key);
        new_env->value = value ? strdup(value) : NULL;
        new_env->next = NULL;
        *env = new_env;
        return;
    }
    
    while (current)
    {
        if (ft_strncmp(current->key, key, ft_strlen(key)) == 0 && 
            ft_strlen(current->key) == ft_strlen(key))
        {
            if (value)
            {      
                free(current->value);
                current->value = strdup(value);
            }
            return;
        }
        current = current->next;
    }
    
    t_env *new_env = malloc(sizeof(t_env));
    if (!new_env)
        return;
    
    // Find the end of the list
    current = *env;
    while (current && current->next)
        current = current->next;
        
    new_env->key = strdup(key);
    if (value)
        new_env->value = strdup(value);
    else
        new_env->value = NULL;
    new_env->next = NULL;
    
    if (!current) // This should never happen since we checked !*env above
    {
        *env = new_env; 
    }
    else
        current->next = new_env;
}

void print_env(t_env *env)
{
    if (!env)
        return;

    t_env *current = env;
    while (current)
    {
        if (current->key && current->value)
            printf("%s=%s\n", current->key, current->value);
        else if (current->key)
            printf("%s=\n", current->key);
        current = current->next;
    }
}
