# Directories
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include
LIB_DIR = lib

# Source files with new directory structure
SRCS = 	$(SRC_DIR)/core/minishell.c \
		$(SRC_DIR)/core/signals.c  \
		$(SRC_DIR)/builtins/env.c \
		$(SRC_DIR)/builtins/pwd.c \
		$(SRC_DIR)/builtins/cd.c \
		$(SRC_DIR)/builtins/export.c \
		$(SRC_DIR)/builtins/unset.c \
		$(SRC_DIR)/execution/execution_memory.c \
		$(SRC_DIR)/execution/execution_path.c \
		$(SRC_DIR)/execution/execution_path_helper.c \
		$(SRC_DIR)/execution/execution_error.c \
		$(SRC_DIR)/execution/execution_process.c \
		$(SRC_DIR)/execution/execution_process_helper.c \
		$(SRC_DIR)/execution/execution_main.c \
		$(SRC_DIR)/execution/execution_pipeline.c \
		$(SRC_DIR)/execution/execution_pipeline_child.c \
		$(SRC_DIR)/execution/execution_pipeline_helper.c \
		$(SRC_DIR)/redirections/redirections.c \
		$(SRC_DIR)/tokenizer/tokenizer_core.c \
		$(SRC_DIR)/tokenizer/tokenizer_variables.c \
		$(SRC_DIR)/tokenizer/tokenizer_quotes.c \
		$(SRC_DIR)/tokenizer/tokenizer_main.c \
		$(SRC_DIR)/parsing/parsing_core.c \
		$(SRC_DIR)/parsing/parsing_tokens.c \
		$(SRC_DIR)/parsing/parsing_redirections.c \
		$(SRC_DIR)/parsing/parsing_utils.c \
		$(SRC_DIR)/redirections/input.c \
		$(SRC_DIR)/redirections/output.c \
		$(SRC_DIR)/utils/env_manager.c \
		$(SRC_DIR)/utils/command_manager.c \
		$(SRC_DIR)/utils/redirection_utils.c \
		$(SRC_DIR)/utils/file_operations.c \
		$(SRC_DIR)/utils/debug_utils.c 

NAME = minishell

# Object files in obj directory, maintaining subdirectory structure
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CC = cc
CFLAGS = -Wall -Wextra -Werror -I$(INC_DIR)
RM = rm -f

all: ${NAME}

bonus: ${NAME_BONUS}

# Create obj directories
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/core
	@mkdir -p $(OBJ_DIR)/builtins
	@mkdir -p $(OBJ_DIR)/execution
	@mkdir -p $(OBJ_DIR)/redirections
	@mkdir -p $(OBJ_DIR)/tokenizer
	@mkdir -p $(OBJ_DIR)/parsing
	@mkdir -p $(OBJ_DIR)/utils

# Compile .c files to .o files in obj directory
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	${CC} ${CFLAGS} -c $< -o $@

.SECONDARY: ${OBJS} ${OBJS_BONUS}

${NAME}: ${OBJS}
	echo "Compiling LIBFT..."
	@make -C ./$(LIB_DIR)/libft
	${CC} ${CFLAGS} ${OBJS} $(LIB_DIR)/libft/libft.a -o ${NAME} -lreadline

clean:
	@make -C ./$(LIB_DIR)/libft clean
	${RM} -r ${OBJ_DIR}

fclean: clean
	@make -C ./$(LIB_DIR)/libft fclean
	${RM} ${NAME} ${NAME_BONUS}

valgrind: ${NAME}
	valgrind --leak-check=full --show-leak-kinds=all --track-fds=all --suppressions=ll.sup ./minishell


re: fclean all