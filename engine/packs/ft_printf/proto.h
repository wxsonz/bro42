#ifndef FT_PRINTF_PROTO_H
# define FT_PRINTF_PROTO_H

/*
** The SUBJECT's prototype, declared by the engine rather than read from the
** student's ft_printf.h - see engine/packs/libft/proto.h for why: the header
** may not exist yet, and if their prototype disagrees with the subject the
** link fails here, which is itself a finding, not something to paper over.
**
** One symbol for the whole pack (_dev/plan/rank01/ftprintf-01-cases.md,
** "How to read this document"): fifteen suites all call this one function.
*/

int	ft_printf(const char *format, ...);

#endif
