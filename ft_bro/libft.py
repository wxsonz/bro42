"""The subject's function inventory, in one place.

Part is not a difficulty ordering - it is the PERMISSION boundary. Subject IV.3
lets Part 2 call write; Parts 1 and 3 may not. The macro symbol audit needs the
part of every function to know which undefined symbols are legal.
"""

PART1 = [
    "ft_isalpha", "ft_isdigit", "ft_isalnum", "ft_isascii", "ft_isprint",
    "ft_strlen", "ft_memset", "ft_bzero", "ft_memcpy", "ft_memmove",
    "ft_strlcpy", "ft_strlcat", "ft_toupper", "ft_tolower", "ft_strchr",
    "ft_strrchr", "ft_strncmp", "ft_memchr", "ft_memcmp", "ft_strnstr",
    "ft_atoi", "ft_calloc", "ft_strdup",
]

PART2 = [
    "ft_substr", "ft_strjoin", "ft_strtrim", "ft_split", "ft_itoa",
    "ft_strmapi", "ft_striteri", "ft_putchar_fd", "ft_putstr_fd",
    "ft_putendl_fd", "ft_putnbr_fd",
]

PART3 = [
    "ft_lstnew", "ft_lstadd_front", "ft_lstsize", "ft_lstlast",
    "ft_lstadd_back", "ft_lstdelone", "ft_lstclear", "ft_lstiter", "ft_lstmap",
]

# Subject 19.2 puts Part 3 in Chapter IV, the MANDATORY part - "IV.4 Part 3 -
# linked list". Libft has no bonus part at all: every "bonus" in the document
# is in the Chapter II boilerplate that ships with every 42 subject.
#
# Repositories written against an older subject put these nine in _bonus.c
# files and gate them behind a `bonus` rule. Both layouts are accepted (see
# macro.source_of); neither is warned about, because the naming is a matter of
# which subject revision the student started under, not of correctness.
MANDATORY = PART1 + PART2 + PART3
ALL = MANDATORY
PART_OF = {**{f: 1 for f in PART1}, **{f: 2 for f in PART2},
           **{f: 3 for f in PART3}}

# Subject IV.2 / IV.3 / IV.4. Calls between your own ft_* functions are legal
# and common - ft_strjoin calling ft_strlen is not a violation - so the real
# allowlist is (these) plus every ft_* name.
ALLOWED_EXTERNALS = {
    1: {"malloc", "free"},
    2: {"malloc", "free", "write"},
    3: {"malloc", "free"},
}
