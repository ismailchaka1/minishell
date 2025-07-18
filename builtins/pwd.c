/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pwd.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 22:44:27 by root              #+#    #+#             */
/*   Updated: 2025/06/17 22:47:38 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../minishell.h"

int builtin_pwd(t_shell *shell)
{
    char cwd[1024];
    if (!shell)
        return 1;
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd");
        return 1;
    }
    printf("%s\n", cwd);
    return 0;
}
