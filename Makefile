# ft_bro - engine objects. The final link happens per target, in bro/build.py,
# because bro_micro is linked against the student's libft.a.

CC       = cc
# -MMD -MP emits a .d per object listing the headers it includes, so touching
# bro.h rebuilds everything that uses it. Without this, changing a struct
# leaves stale objects that disagree about its size - which is exactly the
# dependency-tracking failure plan/platform/08-macro.md section 3 checks for in a student's
# Makefile, and it cost an afternoon here before the check was applied to us.
CFLAGS   = -Wall -Wextra -Werror -g3 -Iengine/include -DBRO_HAVE_WRAP -MMD -MP
OBJDIR   = build

CORE     = util assert guard ctx alloc capture list oracle harness emit registry main
TESTS    = part1/test_ft_isalpha part1/test_ft_isdigit part1/test_ft_isalnum part1/test_ft_isascii part1/test_ft_isprint part1/test_ft_toupper part1/test_ft_tolower \
           part1/test_ft_strlen part1/test_ft_strchr part1/test_ft_strrchr part1/test_ft_strncmp part1/test_ft_memchr part1/test_ft_strnstr part1/test_ft_atoi \
           part1/test_ft_memcmp part1/test_ft_memmove \
           part1/test_ft_memset part1/test_ft_bzero part1/test_ft_memcpy \
           part1/test_ft_strlcpy part1/test_ft_strlcat part1/test_ft_striteri \
           part2/test_ft_itoa part2/test_ft_split part2/test_ft_putnbr_fd \
           part2/test_ft_calloc part2/test_ft_strdup part2/test_ft_substr \
           part2/test_ft_strjoin part2/test_ft_strtrim part2/test_ft_strmapi \
           part2/test_ft_putchar_fd part2/test_ft_putstr_fd part2/test_ft_putendl_fd \
           part3/test_ft_lstclear part3/test_ft_lstnew part3/test_ft_lstadd_front \
           part3/test_ft_lstsize part3/test_ft_lstlast part3/test_ft_lstadd_back \
           part3/test_ft_lstdelone part3/test_ft_lstiter part3/test_ft_lstmap

CORE_SRC = $(addprefix engine/src/,$(addsuffix .c,$(CORE)))
TEST_SRC = $(addprefix engine/tests/,$(addsuffix .c,$(TESTS)))
OBJS     = $(patsubst %.c,$(OBJDIR)/%.o,$(CORE_SRC) $(TEST_SRC))

all: $(OBJS) cases

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

cases:
	@python3 ft_bro/data/gen_cases.py
	@python3 ft_bro/data/gen_concepts.py
	@python3 ft_bro/data/gen_roadmap.py

check:
	@python3 ft_bro/data/gen_cases.py --check
	@python3 ft_bro/data/gen_concepts.py --check
	@python3 ft_bro/data/gen_roadmap.py --check

selftest: all
	@python3 tools/selftest.py

clean:
	@rm -rf $(OBJDIR)

fclean: clean
	@rm -f bro/data/cases.json

re: fclean all

.PHONY: all cases check selftest clean fclean re
