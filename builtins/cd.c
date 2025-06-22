/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 22:47:58 by root              #+#    #+#             */
/*   Updated: 2025/06/22 17:10:32 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int builtin_cd(t_shell *shell, char *path)
{
    char *oldpwd;

    oldpwd = NULL;
    if (!shell)
    {
        fprintf(stderr, "cd: No such file or directory: %s\n", path);
        return 1;
    }

    if (chdir(path) != 0)
    {
        perror("cd");
        return 1;
    }
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        t_env *env = shell->env;
        while (env)
        {
            if (strcmp(env->key, "PWD") == 0)
            {
                oldpwd = strdup(env->value);
                free(env->value);
                env->value = strdup(cwd);
                break;
            }
            env = env->next;
        }
        updateOLDPWD(shell, oldpwd);
    }
    else
    {
        perror("getcwd");
        return 1;
    }
    return 0;
}

int updateOLDPWD(t_shell *shell, char *path)
{
    t_env *env = shell->env;
    while (env)
    {
        if (strcmp(env->key, "OLDPWD") == 0)
        {
            free(env->value);
            env->value = strdup(path);
            free(path);
            return 0;
        }
        env = env->next;
    }
    return 1;
}
