/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichakank <ichakank@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 22:47:58 by root              #+#    #+#             */
/*   Updated: 2025/07/30 18:21:18 by ichakank         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int builtin_cd(t_shell *shell, char **args)
{
    char *oldpwd;

    oldpwd = NULL;
    if (!shell)
    {
        fprintf(stderr, "cd: No such file or directory: %s\n", args[0]);
        return 1;
    }
    if (args && args[1])
        return (fprintf(stderr, "cd: too many arguments\n"), 1);
    if (args && args[0] && args[0][0] == '\0')
        return 0;
    if (!args || (args[0][0] == '~' && args[0][1] == '\0'))
    {
        char *home = getenv("HOME");
        if (!home)
        {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
        if (chdir(home) != 0)
        {
            perror("cd");
            // return 1;
        }
    }
    else if (args && args[0] && chdir(args[0]) != 0)
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
