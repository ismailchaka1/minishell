# Minishell

A simple shell implementation written in C that mimics the behavior of bash.

## Project Structure

```
minishell/
├── src/                    # Source code files
│   ├── core/              # Main program files
│   │   ├── minishell.c    # Main entry point
│   │   └── signals.c      # Signal handling
│   ├── parsing/           # Command parsing
│   │   ├── parsing_core.c       # Core parsing logic
│   │   ├── parsing_tokens.c     # Token handling
│   │   ├── parsing_redirections.c # Redirection parsing
│   │   └── parsing_utils.c      # Parsing utilities
│   ├── tokenizer/         # Input tokenization
│   │   ├── tokenizer_core.c     # Core tokenization
│   │   ├── tokenizer_variables.c # Variable expansion
│   │   ├── tokenizer_quotes.c   # Quote handling
│   │   └── tokenizer_main.c     # Main tokenization logic
│   ├── execution/         # Command execution
│   │   ├── execution_memory.c   # Memory management
│   │   ├── execution_path.c     # Path resolution
│   │   ├── execution_error.c    # Error handling
│   │   ├── execution_process.c  # Process management
│   │   └── execution_main.c     # Main execution logic
│   ├── builtins/          # Built-in commands
│   │   ├── cd.c           # Change directory
│   │   ├── echo.c         # Echo command
│   │   ├── env.c          # Environment variables
│   │   ├── export.c       # Export variables
│   │   ├── pwd.c          # Print working directory
│   │   └── unset.c        # Unset variables
│   ├── utils/             # Utility functions
│   │   ├── command_manager.c    # Command management
│   │   ├── env_manager.c        # Environment management
│   │   ├── debug_utils.c        # Debugging utilities
│   │   ├── file_operations.c    # File operations
│   │   └── redirection_utils.c  # Redirection utilities
│   └── redirections/      # I/O redirection handling
│       ├── redirections.c # Main redirection logic
│       ├── input.c        # Input redirection
│       └── output.c       # Output redirection
├── include/               # Header files
│   └── minishell.h       # Main header file
├── lib/                   # External libraries
│   └── libft/            # Custom C library
├── obj/                   # Object files (generated)
├── docs/                  # Documentation
│   └── en.subject_54.pdf # Project subject
└── Makefile              # Build configuration
```

## Features

- **Command Execution**: Execute system commands and built-in commands
- **Pipes**: Support for command chaining with `|`
- **Redirections**: Input (`<`) and output (`>`, `>>`) redirection
- **Variable Expansion**: Environment variable expansion with `$VAR`
- **Quote Handling**: Single (`'`) and double (`"`) quote support
- **Built-in Commands**: 
  - `cd` - Change directory
  - `echo` - Display text
  - `env` - Display environment variables
  - `export` - Set environment variables
  - `pwd` - Print working directory
  - `unset` - Remove environment variables
- **Signal Handling**: Proper handling of Ctrl+C and other signals

## Building

```bash
make
```

## Cleaning

```bash
make clean    # Remove object files
make fclean   # Remove all generated files
```

## Usage

```bash
./minishell
```

Once started, you can use it like a regular shell:

```bash
minishell> echo "Hello World"
Hello World
minishell> ls -la | grep minishell
-rwxr-xr-x 1 user user 347112 Aug 15 16:29 minishell
minishell> export MY_VAR="test"
minishell> echo $MY_VAR
test
minishell> exit
```

## Development

The project follows a modular structure with each module containing a maximum of 5 functions per file for better maintainability. All source files are organized by functionality:

- **Core**: Main program logic and signal handling
- **Parsing**: Command line parsing and validation
- **Tokenizer**: Input tokenization and variable expansion
- **Execution**: Command execution and process management
- **Built-ins**: Implementation of shell built-in commands
- **Utils**: Shared utility functions
- **Redirections**: Input/output redirection handling

## Memory Management

The project uses careful memory management with proper allocation and deallocation to prevent memory leaks. All dynamically allocated memory is tracked and freed appropriately.

## Error Handling

Comprehensive error handling is implemented throughout the codebase to handle various edge cases and provide meaningful error messages to users.
