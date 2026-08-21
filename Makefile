NAME		= bin/ft_bro_bin
STUB_LIB	= bin/libstubs.a
CC			= cc
CFLAGS		= -Wall -Wextra -Werror -g3 -Iinclude
LIBFT_DIR	?= _dev/reference/libft42git
LIBFT_INC	= -I$(LIBFT_DIR)
LIBFT_LIB	= $(LIBFT_DIR)/libft.a

CORE_SRCS	= src/main.c \
			  src/framework/runner.c \
			  src/framework/assertions.c \
			  src/framework/macro_runner.c \
			  src/framework/malloc_hook.c \
			  src/framework/json_reporter.c \
			  src/framework/browser_launcher.c \
			  src/framework/utils.c \
			  src/tests_part1/test_isalpha.c \
			  src/tests_part1/test_isdigit.c \
			  src/tests_part1/test_isalnum.c \
			  src/tests_part1/test_isascii.c \
			  src/tests_part1/test_isprint.c \
			  src/tests_part1/test_strlen.c \
			  src/tests_part1/test_memset.c \
			  src/tests_part1/test_bzero.c \
			  src/tests_part1/test_memcpy.c \
			  src/tests_part1/test_memmove.c \
			  src/tests_part1/test_strlcpy.c \
			  src/tests_part1/test_strlcat.c \
			  src/tests_part1/test_toupper.c \
			  src/tests_part1/test_tolower.c \
			  src/tests_part1/test_strchr.c \
			  src/tests_part1/test_strrchr.c \
			  src/tests_part1/test_strncmp.c \
			  src/tests_part1/test_memchr.c \
			  src/tests_part1/test_memcmp.c \
			  src/tests_part1/test_strnstr.c \
			  src/tests_part1/test_atoi.c \
			  src/tests_part1/test_calloc.c \
			  src/tests_part1/test_strdup.c \
			  src/tests_part2/test_substr.c \
			  src/tests_part2/test_strjoin.c \
			  src/tests_part2/test_strtrim.c \
			  src/tests_part2/test_split.c \
			  src/tests_part2/test_itoa.c \
			  src/tests_part2/test_strmapi.c \
			  src/tests_part2/test_striteri.c \
			  src/tests_part2/test_putchar_fd.c \
			  src/tests_part2/test_putstr_fd.c \
			  src/tests_part2/test_putendl_fd.c \
			  src/tests_part2/test_putnbr_fd.c \
			  src/tests_part3/test_lstnew.c \
			  src/tests_part3/test_lstadd_front.c \
			  src/tests_part3/test_lstsize.c \
			  src/tests_part3/test_lstlast.c \
			  src/tests_part3/test_lstadd_back.c \
			  src/tests_part3/test_lstdelone.c \
			  src/tests_part3/test_lstclear.c \
			  src/tests_part3/test_lstiter.c \
			  src/tests_part3/test_lstmap.c

STUB_SRC	= src/framework/weak_stubs.c

CORE_OBJS	= $(CORE_SRCS:.c=.o)
STUB_OBJ	= $(STUB_SRC:.c=.o)
ALL_OBJS	= $(CORE_OBJS) $(STUB_OBJ)

all: build_libft $(STUB_LIB) $(NAME)

build_objs: $(ALL_OBJS) $(STUB_LIB)

build_libft:
	@if [ -d "$(LIBFT_DIR)" ]; then \
		make -C $(LIBFT_DIR) bonus >/dev/null 2>&1 || make -C $(LIBFT_DIR) all >/dev/null 2>&1 || true; \
	fi

$(STUB_LIB): $(STUB_OBJ)
	@mkdir -p bin
	@ar rcs $(STUB_LIB) $(STUB_OBJ)

$(NAME): $(CORE_OBJS) $(STUB_LIB)
	@mkdir -p bin
	@if [ -f "$(LIBFT_LIB)" ]; then \
		$(CC) $(CFLAGS) $(CORE_OBJS) $(LIBFT_LIB) $(STUB_LIB) -o $(NAME); \
	else \
		$(CC) $(CFLAGS) $(CORE_OBJS) $(STUB_LIB) -o $(NAME); \
	fi
	@chmod +x bro 2>/dev/null || true
	@echo "\033[1;32m[OK] ft_bro built successfully: $(NAME)\033[0m"

link: $(CORE_OBJS) $(STUB_LIB)
	@mkdir -p bin
	@if [ -f "$(LIBFT_LIB)" ]; then \
		$(CC) $(CFLAGS) $(CORE_OBJS) $(LIBFT_LIB) $(STUB_LIB) -o $(NAME); \
	else \
		$(CC) $(CFLAGS) $(CORE_OBJS) $(STUB_LIB) -o $(NAME); \
	fi

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LIBFT_INC) -c $< -o $@

clean:
	@rm -f $(ALL_OBJS)

fclean: clean
	@rm -rf bin

re: fclean all

.PHONY: all build_objs build_libft link clean fclean re
