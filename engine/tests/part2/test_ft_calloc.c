#include "bro.h"
#include "libft_proto.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
** ft_calloc - T2. Cases from _dev/SPEC_MICRO.md section 22.
**
** Cases 1-2 are the only two that carry INJECT: they are the only ones that
** make it past the size checks and actually allocate through the wrapped
** allocator, so they are the only ones a failure can be injected into.
** Cases 3-7 test the return contract at the boundaries (Subject IV.2's
** zero-size guarantee, and the multiplication overflow check) and always
** either succeed or return NULL outright - there is no allocation sequence
** for the harness to sweep.
**
** Cases 6 and 7 are NOT equivalent. SIZE_MAX * 2 wraps to a number still far
** too large for malloc to satisfy, so an implementation with no overflow
** check at all passes case 6 by luck - malloc just fails on its own. Case 7
** is the real check: (SIZE_MAX / 2 + 1) * 2 wraps to exactly 0, so an
** unchecked implementation hands back a valid zero-byte block instead of the
** NULL the overflow demands. Both cases stay for that reason.
*/

static void	calloc_zeroed_case(t_ctx *c, size_t nmemb, size_t size,
		size_t total)
{
	unsigned char	*got;
	size_t			i;

	got = ft_calloc(nmemb, size);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	i = 0;
	while (i < total)
	{
		if (got[i] != 0)
		{
			bro_fail(c->out, "byte %zu was not zeroed", i);
			return (free(got));
		}
		got[i] = 0xff;
		i++;
	}
	free(got);
}

/* Subject IV.2: a zero-size request still returns a unique, freeable block. */
static void	calloc_alive_case(t_ctx *c, size_t nmemb, size_t size)
{
	void	*got;

	got = ft_calloc(nmemb, size);
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	free(got);
}

static void	calloc_null_case(t_ctx *c, size_t nmemb, size_t size)
{
	void	*got;

	got = ft_calloc(nmemb, size);
	if (got)
	{
		free(got);
		return (bro_fail(c->out, "expected NULL, got a non-NULL pointer"));
	}
}

static void	case_01(t_ctx *c) { calloc_zeroed_case(c, 5, sizeof(int), 20); }
static void	case_02(t_ctx *c) { calloc_zeroed_case(c, 1, 1, 1); }
static void	case_03(t_ctx *c) { calloc_alive_case(c, 0, 0); }
static void	case_04(t_ctx *c) { calloc_alive_case(c, 0, 10); }
static void	case_05(t_ctx *c) { calloc_alive_case(c, 10, 0); }
static void	case_06(t_ctx *c) { calloc_null_case(c, SIZE_MAX, 2); }
static void	case_07(t_ctx *c) { calloc_null_case(c, SIZE_MAX / 2 + 1, 2); }

/*
** Large but entirely valid. An overflow guard written against a fixed ceiling
** instead of SIZE_MAX rejects this, and nothing else in the suite would notice.
*/
static void	case_08(t_ctx *c) { calloc_zeroed_case(c, 100000, 1, 100000); }

static const t_case	g_cases[] = {
{1, "ft_calloc(5, sizeof(int))", BRO_ORACLE | BRO_INJECT, case_01},
{2, "ft_calloc(1, 1)", BRO_ORACLE | BRO_INJECT, case_02},
{3, "ft_calloc(0, 0)", BRO_ORACLE, case_03},
{4, "ft_calloc(0, 10)", BRO_ORACLE, case_04},
{5, "ft_calloc(10, 0)", BRO_ORACLE, case_05},
{6, "ft_calloc(SIZE_MAX, 2)", BRO_ORACLE, case_06},
{7, "ft_calloc(SIZE_MAX / 2 + 1, 2)", BRO_ORACLE, case_07},
{8, "ft_calloc(100000, 1)", BRO_ORACLE, case_08},
};

const t_suite	g_suite_ft_calloc = {
	"ft_calloc", 1, 5, 2, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
