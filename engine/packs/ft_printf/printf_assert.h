#ifndef PRINTF_ASSERT_H
# define PRINTF_ASSERT_H

# include "bro.h"
# include <stdbool.h>

/*
** Shared oracle/assert helper for every ft_printf suite - all fifteen are
** Tier T4 (_dev/plan/rank01/ftprintf-01-cases.md, "How to read this
** document": "this pack has one observable"). See printf_assert.c for the
** design.
*/

bool	pf_begin(t_ctx *c);
void	pf_check(t_ctx *c, int actual_ret, const char *expected,
			int expected_len);

#endif
