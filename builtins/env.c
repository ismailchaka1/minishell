/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 21:35:00 by root              #+#    #+#             */
/*   Updated: 2025/06/17 22:41:59 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

// Built-in env command - prints all environment variables
int builtin_env(t_shell *shell)
{    
    if (!shell || !shell->env)
        return 1;
    
    print_env(shell->env);
    return 0;
}
