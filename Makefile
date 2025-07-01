SRCS = 	minishell.c signals.c builtins/env.c builtins/pwd.c builtins/cd.c execution.c redirections.c
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

re: fclean all
