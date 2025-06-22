t_command *parse_tokens(t_token *tokens)
{
    t_command *head = NULL;
    t_command *current = NULL;

    while (tokens && tokens->type != TOKEN_EOF)
    {
        // // handle pipes and redirections and heredocs
        // if (tokens->type == TOKEN_PIPE)
        // {
        //     // Create a new command for the pipeline
        //     t_command *new_cmd = malloc(sizeof(t_command));
        //     if (!new_cmd)
        //         return NULL;
        //     new_cmd->args = NULL;
        //     new_cmd->input_file = NULL;
        //     new_cmd->output_file = NULL;
        //     new_cmd->append = false;
        //     new_cmd->heredoc = false;
        //     new_cmd->next = NULL;

        //     if (!head)
        //     {
        //         head = new_cmd;
        //         current = head;
        //     }
        //     else
        //     {
        //         current->next = new_cmd;
        //         current = new_cmd;
        //     }
        // }
        // else if (tokens->type == TOKEN_REDIRECT_IN || 
        //          tokens->type == TOKEN_REDIRECT_OUT || 
        //          tokens->type == TOKEN_APPEND || 
        //          tokens->type == TOKEN_HEREDOC)
        // {
        //     // Handle redirection
        //     if (!current)
        //     {
        //         printf("Error: Redirection without command\n");
        //         return NULL;
        //     }
        //     if (tokens->type == TOKEN_REDIRECT_IN)
        //         current->input_file = strdup(tokens->next ? tokens->next->value : "");
        //     else if (tokens->type == TOKEN_REDIRECT_OUT)
        //         current->output_file = strdup(tokens->next ? tokens->next->value : "");
        //     else if (tokens->type == TOKEN_APPEND)
        //     {
        //         current->output_file = strdup(tokens->next ? tokens->next->value : "");
        //         current->append = true;
        //     }
        //     else if (tokens->type == TOKEN_HEREDOC)
        //     {
        //         current->heredoc = true;
        //         current->input_file = strdup(tokens->next ? tokens->next->value : "");
        //     }
        //     // Skip the next token as it is the file name for redirection
        //     if (tokens->next)
        //         tokens = tokens->next;
        // }
        // else if (tokens->type == TOKEN_WORD)
        // {
        //     if (!current)
        //     {
        //         printf("Error: Word without command\n");
        //         return NULL;
        //     }
        //     char **args = realloc(current->args, sizeof(char *) * 
        //                           (current->args ? sizeof(current->args) / sizeof(char *) + 1 : 1));
        //     if (!args)
        //         return NULL;
        //     args[sizeof(args) / sizeof(char *)] = strdup(tokens->value);
        //     current->args = args;
        // }
    }
    return head;
}