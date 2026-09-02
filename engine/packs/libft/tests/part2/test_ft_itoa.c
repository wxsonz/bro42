#include "bro.h"
#include "proto.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/*
** ft_itoa - T2. Cases from _dev/plan/rank00/libft-01-cases.md section 28.
**
** Every case carries INJECT: the harness runs it once to count allocations,
** then re-runs it with malloc failing at each index in turn. itoa makes one
** allocation, so that is one extra check per case, for free.
**
** Case 13 closes a real hole found by writing a wrong implementation that
** mallocs just the digit count, forgetting the +1 for the terminator, and
** watching it score 12/12 on the cases above: the write lands one byte past
** the block, but malloc's own rounding usually absorbs it silently, so the
** text comes out right anyway. Only the allocated byte count, read from the
** wrapped allocator, catches the shortage.
*/

static void	itoa_case(t_ctx *c, int n, const char *expected)
{
	char	*got;

	got = ft_itoa(n);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (strcmp(got, expected))
		bro_fail(c->out, "expected \"%s\", got \"%s\"", expected, got);
	free(got);
}

static void	case_01(t_ctx *c) { itoa_case(c, 0, "0"); }
static void	case_02(t_ctx *c) { itoa_case(c, 42, "42"); }
static void	case_03(t_ctx *c) { itoa_case(c, -42, "-42"); }
static void	case_04(t_ctx *c) { itoa_case(c, -1, "-1"); }
static void	case_05(t_ctx *c) { itoa_case(c, 9, "9"); }
static void	case_06(t_ctx *c) { itoa_case(c, 10, "10"); }
static void	case_07(t_ctx *c) { itoa_case(c, 99, "99"); }
static void	case_08(t_ctx *c) { itoa_case(c, 100, "100"); }
static void	case_09(t_ctx *c) { itoa_case(c, -9, "-9"); }
static void	case_10(t_ctx *c) { itoa_case(c, -10, "-10"); }
static void	case_11(t_ctx *c) { itoa_case(c, INT_MAX, "2147483647"); }
static void	case_12(t_ctx *c) { itoa_case(c, INT_MIN, "-2147483648"); }

static void	case_13(t_ctx *c)
{
	t_alloc	before;
	t_alloc	after;
	char	*got;
	size_t	got_bytes;

	bro_alloc_snapshot(&before);
	got = ft_itoa(-73);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	bro_alloc_snapshot(&after);
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	got_bytes = after.live_bytes - before.live_bytes;
	if (got_bytes < 4)
		bro_fail(c->out, "the result needs at least 4 bytes, "
			"only %zu were allocated", got_bytes);
	free(got);
}

static const t_case	g_cases[] = {
{1, "ft_itoa(0)", BRO_INJECT, case_01},
{2, "ft_itoa(42)", BRO_INJECT, case_02},
{3, "ft_itoa(-42)", BRO_INJECT, case_03},
{4, "ft_itoa(-1)", BRO_INJECT, case_04},
{5, "ft_itoa(9)", BRO_INJECT, case_05},
{6, "ft_itoa(10)", BRO_INJECT, case_06},
{7, "ft_itoa(99)", BRO_INJECT, case_07},
{8, "ft_itoa(100)", BRO_INJECT, case_08},
{9, "ft_itoa(-9)", BRO_INJECT, case_09},
{10, "ft_itoa(-10)", BRO_INJECT, case_10},
{11, "ft_itoa(2147483647)", BRO_INJECT, case_11},
{12, "ft_itoa(-2147483648)", BRO_INJECT, case_12},
{13, "ft_itoa(-73): exact allocation", BRO_INJECT, case_13},
};

const t_suite	g_suite_ft_itoa = {
	"ft_itoa", 2, 5, 2, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
