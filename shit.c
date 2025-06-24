#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    STATE_NORMAL,
    STATE_SINGLE_QUOTE,
    STATE_DOUBLE_QUOTE,
    STATE_ESCAPE
} parse_state_t;

typedef struct {
    char *buffer;
    size_t size;
    size_t capacity;
} dynamic_string_t;

// Dynamic string functions
dynamic_string_t* ds_create() {
    dynamic_string_t *ds = malloc(sizeof(dynamic_string_t));
    ds->capacity = 64;
    ds->buffer = malloc(ds->capacity);
    ds->size = 0;
    ds->buffer[0] = '\0';
    return ds;
}

void ds_append_char(dynamic_string_t *ds, char c) {
    if (ds->size + 1 >= ds->capacity) {
        ds->capacity *= 2;
        ds->buffer = realloc(ds->buffer, ds->capacity);
    }
    ds->buffer[ds->size++] = c;
    ds->buffer[ds->size] = '\0';
}

void ds_free(dynamic_string_t *ds) {
    free(ds->buffer);
    free(ds);
}

// Check if character needs expansion in double quotes
int needs_expansion(char c, const char *next) {
    return (c == '$' || (c == '`') || (c == '\\' && 
           (next && (*next == '"' || *next == '\\' || *next == '$' || *next == '`' || *next == '\n'))));
}

// Simulate variable expansion (simplified)
char* expand_variable(const char **input) {
    const char *start = *input;
    const char *end = start;
    
    if (*end == '{') {
        // ${VAR} form
        end++;
        while (*end && *end != '}') end++;
        if (*end == '}') end++;
    } else {
        // $VAR form
        while (*end && (isalnum(*end) || *end == '_')) end++;
    }
    
    *input = end - 1; // Will be incremented by main loop
    
    // Return mock expansion (in real implementation, look up variable)
    char *result = malloc(16);
    strcpy(result, "EXPANDED");
    return result;
}

// Main quote parsing function
char* parse_bash_quotes(const char *input) {
    if (!input) return NULL;
    
    dynamic_string_t *result = ds_create();
    parse_state_t state = STATE_NORMAL;
    parse_state_t prev_state = STATE_NORMAL;
    
    const char *p = input;
    
    while (*p) {
        char current = *p;
        char next = *(p + 1);
        
        switch (state) {
            case STATE_NORMAL:
                switch (current) {
                    case '\'':
                        state = STATE_SINGLE_QUOTE;
                        // Quote character is removed (quote removal)
                        break;
                    case '"':
                        state = STATE_DOUBLE_QUOTE;
                        // Quote character is removed (quote removal)
                        break;
                    case '\\':
                        prev_state = STATE_NORMAL;
                        state = STATE_ESCAPE;
                        break;
                    case '$':
                        // Variable expansion in normal state
                        p++; // Skip $
                        char *expanded = expand_variable(&p);
                        for (char *e = expanded; *e; e++) {
                            ds_append_char(result, *e);
                        }
                        free(expanded);
                        break;
                    default:
                        ds_append_char(result, current);
                        break;
                }
                break;
                
            case STATE_SINGLE_QUOTE:
                if (current == '\'') {
                    state = STATE_NORMAL;
                    // Quote character is removed (quote removal)
                } else {
                    // Everything is literal inside single quotes
                    ds_append_char(result, current);
                }
                break;
                
            case STATE_DOUBLE_QUOTE:
                switch (current) {
                    case '"':
                        state = STATE_NORMAL;
                        // Quote character is removed (quote removal)
                        break;
                    case '\\':
                        if (next == '"' || next == '\\' || next == '$' || 
                            next == '`' || next == '\n') {
                            prev_state = STATE_DOUBLE_QUOTE;
                            state = STATE_ESCAPE;
                        } else {
                            // Backslash is literal if not escaping special char
                            ds_append_char(result, current);
                        }
                        break;
                    case '$':
                        // Variable expansion inside double quotes
                        p++; // Skip $
                        char *expanded = expand_variable(&p);
                        for (char *e = expanded; *e; e++) {
                            ds_append_char(result, *e);
                        }
                        free(expanded);
                        break;
                    case '`':
                        // Command substitution (simplified)
                        ds_append_char(result, 'C');
                        ds_append_char(result, 'M');
                        ds_append_char(result, 'D');
                        break;
                    default:
                        ds_append_char(result, current);
                        break;
                }
                break;
                
            case STATE_ESCAPE:
                // Add the escaped character literally
                ds_append_char(result, current);
                state = prev_state;
                break;
        }
        
        p++;
    }
    
    // Check for unclosed quotes
    if (state == STATE_SINGLE_QUOTE || state == STATE_DOUBLE_QUOTE) {
        fprintf(stderr, "Error: Unclosed quote\n");
        ds_free(result);
        return NULL;
    }
    
    char *final_result = strdup(result->buffer);
    ds_free(result);
    return final_result;
}

// Tokenization function (splits on whitespace, respecting quotes)
typedef struct {
    char **tokens;
    size_t count;
    size_t capacity;
} token_list_t;

token_list_t* tokenize_bash_line(const char *input) {
    if (!input) return NULL;
    
    token_list_t *tokens = malloc(sizeof(token_list_t));
    tokens->capacity = 16;
    tokens->tokens = malloc(tokens->capacity * sizeof(char*));
    tokens->count = 0;
    
    dynamic_string_t *current_token = ds_create();
    parse_state_t state = STATE_NORMAL;
    
    const char *p = input;
    
    while (*p) {
        char current = *p;
        
        // Skip leading whitespace in normal state
        if (state == STATE_NORMAL && isspace(current)) {
            if (current_token->size > 0) {
                // End current token
                if (tokens->count >= tokens->capacity) {
                    tokens->capacity *= 2;
                    tokens->tokens = realloc(tokens->tokens, 
                                           tokens->capacity * sizeof(char*));
                }
                tokens->tokens[tokens->count++] = parse_bash_quotes(current_token->buffer);
                current_token->size = 0;
                current_token->buffer[0] = '\0';
            }
            p++;
            continue;
        }
        
        // Track quote state for tokenization
        switch (current) {
            case '\'':
                if (state == STATE_NORMAL) {
                    state = STATE_SINGLE_QUOTE;
                } else if (state == STATE_SINGLE_QUOTE) {
                    state = STATE_NORMAL;
                }
                ds_append_char(current_token, current);
                break;
            case '"':
                if (state == STATE_NORMAL) {
                    state = STATE_DOUBLE_QUOTE;
                } else if (state == STATE_DOUBLE_QUOTE) {
                    state = STATE_NORMAL;
                }
                ds_append_char(current_token, current);
                break;
            default:
                ds_append_char(current_token, current);
                break;
        }
        
        p++;
    }
    
    // Add final token if exists
    if (current_token->size > 0) {
        if (tokens->count >= tokens->capacity) {
            tokens->capacity *= 2;
            tokens->tokens = realloc(tokens->tokens, 
                                   tokens->capacity * sizeof(char*));
        }
        tokens->tokens[tokens->count++] = parse_bash_quotes(current_token->buffer);
    }
    
    ds_free(current_token);
    return tokens;
}

void free_tokens(token_list_t *tokens) {
    if (!tokens) return;
    for (size_t i = 0; i < tokens->count; i++) {
        free(tokens->tokens[i]);
    }
    free(tokens->tokens);
    free(tokens);
}

// Test function
int main() {
    printf("Bash Quote Parser Test\n");
    printf("======================\n\n");
    
    const char *test_cases[] = {
        "echo 'Hello World'",
        "echo \"Hello $USER\"",
        "echo 'It\\'s working'",
        "echo \"It's \\\"quoted\\\"\"",
        "echo $HOME/file",
        "echo \"$HOME/file\"",
        "echo 'No $expansion here'",
        "echo \"Mixed 'quotes' here\"",
        "echo 'Mixed \"quotes\" here'",
        "echo \"Escaped \\$dollar\"",
        NULL
    };
    
    for (int i = 0; test_cases[i]; i++) {
        printf("Input:  %s\n", test_cases[i]);
        
        token_list_t *tokens = tokenize_bash_line(test_cases[i]);
        printf("Tokens: ");
        for (size_t j = 0; j < tokens->count; j++) {
            printf("[%s] ", tokens->tokens[j]);
        }
        printf("\n\n");
        
        free_tokens(tokens);
    }
    
    return 0;
}