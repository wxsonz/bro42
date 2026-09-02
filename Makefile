# bro42 - engine objects. The final link happens per target, in bro42/build.py,
# because bro_micro is linked against the student's libft.a.

CC       = cc
# -MMD -MP emits a .d per object listing the headers it includes, so touching
# bro.h rebuilds everything that uses it. Without this, changing a struct
# leaves stale objects that disagree about its size - which is exactly the
# dependency-tracking failure plan/platform/08-macro.md section 3 checks for in a student's
# Makefile, and it cost an afternoon here before the check was applied to us.
CFLAGS   = -Wall -Wextra -Werror -g3 -Iengine/include -Iengine/packs/libft -DBRO_HAVE_WRAP -MMD -MP
OBJDIR   = build

# Project-agnostic harness. Knows nothing about Libft, ft_printf, or any
# other pack - see engine/core/.
CORE        = util assert guard ctx alloc capture oracle harness emit main
CORE_SRC    = $(addprefix engine/core/,$(addsuffix .c,$(CORE)))
CORE_OBJS   = $(patsubst engine/core/%.c,$(OBJDIR)/core/%.o,$(CORE_SRC))

# The Libft pack: its own registry/fixtures plus one test file per function.
# A second pack (ft_printf) gets its own PACK_SRC/PACK_OBJS pair alongside
# this one, landing in build/packs/<name>/ so its objects can never be linked
# into another pack's binary.
LIBFT_PACK  = registry list
LIBFT_TESTS = part1/test_ft_isalpha part1/test_ft_isdigit part1/test_ft_isalnum part1/test_ft_isascii part1/test_ft_isprint part1/test_ft_toupper part1/test_ft_tolower \
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

LIBFT_SRC   = $(addprefix engine/packs/libft/,$(addsuffix .c,$(LIBFT_PACK))) \
              $(addprefix engine/packs/libft/tests/,$(addsuffix .c,$(LIBFT_TESTS)))
LIBFT_OBJS  = $(patsubst engine/packs/libft/%.c,$(OBJDIR)/packs/libft/%.o,$(LIBFT_SRC))

# The ft_printf pack: its own registry/oracle-assert helper plus one test
# file per suite. Lands in build/packs/ft_printf/, never build/packs/libft/ -
# see engine/packs/ft_printf/registry.c's top comment for how to add suite #7
# onward. -Iengine/packs/ft_printf is scoped to this pack's own compile rule,
# same as -Iengine/packs/libft is global only because Libft was here first;
# a "proto.h" include always resolves against the including file's own
# directory first, so the two packs' identically-named headers never collide.
FT_PRINTF_PACK  = registry printf_assert
FT_PRINTF_TESTS = mandatory/test_pf_literal mandatory/test_pf_pct mandatory/test_pf_c \
           mandatory/test_pf_s mandatory/test_pf_d mandatory/test_pf_i

FT_PRINTF_SRC   = $(addprefix engine/packs/ft_printf/,$(addsuffix .c,$(FT_PRINTF_PACK))) \
              $(addprefix engine/packs/ft_printf/tests/,$(addsuffix .c,$(FT_PRINTF_TESTS)))
FT_PRINTF_OBJS  = $(patsubst engine/packs/ft_printf/%.c,$(OBJDIR)/packs/ft_printf/%.o,$(FT_PRINTF_SRC))

OBJS     = $(CORE_OBJS) $(LIBFT_OBJS) $(FT_PRINTF_OBJS)

all: $(OBJS) cases

$(OBJDIR)/core/%.o: engine/core/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/packs/libft/%.o: engine/packs/libft/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/packs/ft_printf/%.o: engine/packs/ft_printf/%.c
	@mkdir -p $(dir $@)
	$(CC) -Iengine/packs/ft_printf $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

# PACK selects which project's spec to regenerate from - default libft.
# `make cases PACK=ft_printf` needs _dev/plan/rank01's spec, which the
# platform doc (10-packs.md) explicitly leaves for a later pass.
PACK ?= libft

cases:
	@python3 bro42/data/gen_cases.py --pack $(PACK)
	@python3 bro42/data/gen_concepts.py --pack $(PACK)
	@python3 bro42/data/gen_roadmap.py --pack $(PACK)

check:
	@python3 bro42/data/gen_cases.py --pack $(PACK) --check
	@python3 bro42/data/gen_concepts.py --pack $(PACK) --check
	@python3 bro42/data/gen_roadmap.py --pack $(PACK) --check

selftest: all
	@python3 tools/selftest.py

clean:
	@rm -rf $(OBJDIR)

fclean: clean
	@rm -f bro42/packs/$(PACK)/data/cases.json bro42/packs/$(PACK)/data/concepts.json \
	       bro42/packs/$(PACK)/data/roadmap.json

re: fclean all

.PHONY: all cases check selftest clean fclean re
