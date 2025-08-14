SRCS = 	minishell.c \
		signals.c  \
		builtins/env.c \
		builtins/pwd.c \
		builtins/cd.c \
		builtins/export.c \
		builtins/unset.c \
		execution.c \
		redirections.c \
		tokenizer.c \
		parsing.c \
		redirections/input.c \
		redirections/output.c \
		env_manager.c \
		command_manager.c \
		redirection_utils.c \
		file_operations.c \
		debug_utils.c 
NAME = minishell
OBJS = ${SRCS:.c=.o}
CC = cc
CFLAGS = -Wall -Wextra -Werror -fsanitize=address -g3
RM = rm -f

all: ${NAME}

bonus: ${NAME_BONUS}

.SECONDARY: ${OBJS} ${OBJS_BONUS}

${NAME}: ${OBJS}
	echo "Compiling LIBFT..."
	@make -C ./libft
	${CC} ${CFLAGS} ${OBJS} libft/libft.a -o ${NAME} -lreadline

clean:
	@make -C ./libft clean
	${RM} ${OBJS} ${OBJS_BONUS}

fclean: clean
	@make -C ./libft fclean
	${RM} ${NAME} ${NAME_BONUS}

valgrind: ${NAME}
	valgrind --leak-check=full --show-leak-kinds=all --track-fds=all --suppressions=ll.sup ./minishell


re: fclean all
