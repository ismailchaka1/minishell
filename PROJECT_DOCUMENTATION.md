# Minishell Project Documentation

## Overview

This is a comprehensive implementation of a Unix shell (minishell) written in C. The project recreates the functionality of a basic shell like bash, providing command execution, environment variable management, redirection, pipes, signal handling, and built-in commands.

## Project Structure

```
minishell/
├── minishell.c          # Main source file with core shell logic
├── minishell.h          # Header file with all structure definitions
├── execution.c          # Command execution and pipeline handling
├── redirections.c       # Input/Output redirection implementations
├── signals.c           # Signal handling (Ctrl+C, Ctrl+\, etc.)
├── parsing.c           # Token parsing (currently empty)
├── Makefile            # Build configuration
├── builtins/           # Built-in commands directory
│   ├── cd.c            # Change directory implementation
│   ├── env.c           # Environment variable display
│   └── pwd.c           # Print working directory
└── libft/              # Custom C library with utility functions
    ├── libft.h         # Library header
    ├── Makefile        # Library build file
    └── *.c files       # Various utility functions
```

## Core Features

### 1. Command Line Interface
- **Interactive Prompt**: Displays `minishell > ` and waits for user input
- **History Support**: Uses GNU readline library for command history and line editing
- **Signal Handling**: Properly handles Ctrl+C (SIGINT) and Ctrl+\ (SIGQUIT)

### 2. Tokenization and Parsing

#### Token Types
The shell recognizes several token types:
```c
typedef enum e_token_type {
    TOKEN_WORD,          // Commands, arguments, or filenames
    TOKEN_PIPE,          // |
    TOKEN_REDIRECT_IN,   // <
    TOKEN_REDIRECT_OUT,  // >
    TOKEN_APPEND,        // >>
    TOKEN_HEREDOC,       // <<
    TOKEN_SINGLE_QUOTE,  // 'string'
    TOKEN_DOUBLE_QUOTE,  // "string"
    TOKEN_EOF            // End of input
} t_token_type;
```

#### Tokenization Process
1. **Whitespace Handling**: Skips leading/trailing spaces
2. **Operator Recognition**: Identifies pipes and redirection operators
3. **Quote Processing**: Handles single and double quotes with proper escaping
4. **Variable Expansion**: Expands environment variables (`$VAR`, `${VAR}`, `$?`)
5. **Word Splitting**: Splits unquoted variables by whitespace

### 3. Variable Expansion

The shell supports various forms of variable expansion:
- **Simple Variables**: `$HOME`, `$PATH`
- **Braced Variables**: `${HOME}`, `${USER}`
- **Exit Status**: `$?` expands to the last command's exit status
- **Quote Handling**: Variables expand inside double quotes but not single quotes

### 4. Command Execution

#### Built-in Commands
- **`cd`**: Changes working directory, supports `~` for home directory
- **`pwd`**: Prints current working directory
- **`env`**: Displays all environment variables
- **`exit`**: Exits the shell

#### External Commands
- **PATH Resolution**: Searches for commands in PATH directories
- **Process Creation**: Uses `fork()` and `execve()` for command execution
- **Environment Passing**: Converts internal environment to format suitable for `execve()`

### 5. Redirection

#### Input Redirection (`<`)
```c
int handle_command_input_redirection(t_redirect *redirect) {
    int fd = open(redirect->filename, O_RDONLY);
    if (fd == -1) {
        perror(redirect->filename);
        return -1;
    }
    if (dup2(fd, STDIN_FILENO) == -1) {
        perror("dup2");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}
```

#### Output Redirection (`>` and `>>`)
- **Truncate Mode (`>`)**: Creates or truncates the output file
- **Append Mode (`>>`)**: Appends to existing file or creates new one

#### Here Document (`<<`)
```c
int handle_commmand_herdoc(t_redirect *redirect) {
    int fd = open(".heredoc", O_RDWR | O_CREAT | O_TRUNC, 0644);
    printf("Heredoc started, type '%s' to end input\n", redirect->filename);
    while (1) {
        input = readline("heredoc > ");
        if (!input) break;
        
        if (ft_strncmp(input, redirect->filename, ft_strlen(redirect->filename)) == 0) {
            free(input);
            break;
        }
        write(fd, input, strlen(input));
        write(fd, "\n", 1);
        free(input);
    }
    // Redirect to stdin and cleanup
    lseek(fd, 0, SEEK_SET);
    dup2(fd, STDIN_FILENO);
    close(fd);
    unlink(".heredoc");
    return 0;
}
```

### 6. Pipeline Support

The shell supports command pipelines (`command1 | command2 | command3`):
- **Pipe Creation**: Creates pipes between commands
- **Process Coordination**: Manages multiple processes in pipeline
- **Data Flow**: Connects stdout of one command to stdin of the next

### 7. Signal Handling

#### Interactive Mode
```c
static void sigint_handler(int signum) {
    (void)signum;
    write(1, "\n", 1);
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}
```

#### Execution Mode
- **SIGINT**: Handled during command execution
- **SIGQUIT**: Properly ignored in interactive mode

### 8. Environment Management

#### Environment Structure
```c
typedef struct s_env {
    char *key;          // Environment variable key
    char *value;        // Environment variable value
    struct s_env *next; // Pointer to next environment variable
} t_env;
```

#### Shell State
```c
typedef struct s_shell {
    t_env *env;          // Environment variables
    int exit_status;     // Last command exit status
} t_shell;
```

## Data Structures

### Command Structure
```c
typedef struct s_command {
    char *command;          // Command to execute
    char **args;            // Arguments for the command
    t_redirect *redirects;  // List of all redirections
    char *input_file;       // Input redirection file
    char *output_file;      // Output redirection file
    bool append;            // Append mode for output redirection
    bool heredoc;           // Indicates if this command uses heredoc
    char *heredoc_delimiter; // Delimiter for heredoc
    char *path;            // Full path to the command
    struct s_command *next; // Pointer to the next command in pipeline
} t_command;
```

### Redirection Structure
```c
typedef struct s_redirect {
    char *filename;         // Redirection filename
    int type;              // 0=input, 1=output, 2=append, 3=heredoc
    bool quoted_delimiter; // For heredoc: true if delimiter was quoted
    struct s_redirect *next;
} t_redirect;
```

## Build System

### Makefile Configuration
```makefile
SRCS = minishell.c signals.c builtins/env.c builtins/pwd.c builtins/cd.c execution.c redirections.c
NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror -fsanitize=address -g3
```

### Dependencies
- **GNU Readline**: For command line editing and history
- **Custom libft**: Utility functions for string manipulation, memory management
- **System Libraries**: Standard C libraries for file operations, process management

### Compilation
```bash
make          # Build the project
make clean    # Remove object files
make fclean   # Remove all generated files
make re       # Clean and rebuild
```

## Memory Management

### Allocation Strategy
- **Dynamic Allocation**: All strings and structures are dynamically allocated
- **Proper Cleanup**: Each allocation has corresponding free operations
- **Error Handling**: Allocation failures are properly handled

### Memory Leak Prevention
- **Command Cleanup**: `free_commands()` recursively frees command structures
- **Token Cleanup**: `free_tokenizer()` cleans up tokenization data
- **Environment Cleanup**: `free_env()` releases environment variables

## Error Handling

### Input Validation
- **Empty Commands**: Properly handles empty input
- **Invalid Syntax**: Detects and reports syntax errors
- **File Permissions**: Checks file accessibility before operations

### Runtime Errors
- **Command Not Found**: Returns appropriate exit status (127)
- **Permission Denied**: Handles file permission errors
- **System Call Failures**: Proper error reporting for system calls

## Advanced Features

### Quote Processing
- **Single Quotes**: Preserves literal values
- **Double Quotes**: Allows variable expansion
- **Escape Sequences**: Handles backslash escaping

### Variable Expansion Edge Cases
- **Undefined Variables**: Expand to empty string
- **Nested Expansion**: Supports complex variable references
- **Word Splitting**: Proper handling of expanded variables

### Redirection Combinations
- **Multiple Redirections**: Supports multiple redirections per command
- **Redirection Order**: Processes redirections in order
- **Error Recovery**: Restores original file descriptors on errors

## Known Issues and Limitations

### Current Implementation Status
1. **parsing.c**: File exists but is empty - parsing logic is in minishell.c
2. **Pipeline Execution**: Basic implementation present but may need refinement
3. **Complex Quoting**: Some edge cases in quote handling may need attention
4. **Heredoc with Variables**: Variable expansion in heredoc needs verification

### Areas for Improvement
1. **Code Organization**: Some parsing logic could be moved to parsing.c
2. **Error Messages**: Could be more specific and user-friendly
3. **Performance**: Some string operations could be optimized
4. **Testing**: More comprehensive test cases needed

## Usage Examples

### Basic Commands
```bash
minishell > ls -la
minishell > pwd
minishell > cd /tmp
minishell > env
```

### Redirection
```bash
minishell > echo "Hello" > output.txt
minishell > cat < input.txt
minishell > ls >> logfile.txt
minishell > cat << EOF
```

### Pipelines
```bash
minishell > ls | grep txt
minishell > ps aux | grep minishell | wc -l
```

### Variable Usage
```bash
minishell > echo $HOME
minishell > echo "User: $USER"
minishell > echo Exit status: $?
```

## Development Guidelines

### Code Style
- **Function Naming**: Clear, descriptive function names
- **Error Handling**: Consistent error checking and reporting
- **Memory Management**: Always pair malloc with free
- **Comments**: Detailed comments for complex logic

### Testing Strategy
- **Unit Testing**: Test individual functions
- **Integration Testing**: Test command combinations
- **Edge Cases**: Test unusual input scenarios
- **Memory Testing**: Use valgrind to check for leaks

This minishell implementation provides a solid foundation for understanding shell internals and can be extended with additional features like job control, command substitution, and more advanced scripting capabilities.
