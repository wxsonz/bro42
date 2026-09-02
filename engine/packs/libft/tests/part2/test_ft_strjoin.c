#include "bro.h"
#include "proto.h"
#include <stdlib.h>
#include <string.h>

/*
** ft_strjoin - T2. Cases from _dev/plan/rank00/libft-01-cases.md section 25.
**
** Cases 6 and 7 are about ownership: strjoin takes char const * and reads
** its arguments, so both must come back unmodified and still valid, and the
** result must be a distinct pointer from both. Each holds its own copy in a
** mutable local buffer to compare against - a string literal input would
** not let a bug corrupt it.
**
** Case 8 closes a real hole found by writing a wrong implementation that
** mallocs strlen(s1) + strlen(s2), forgetting the +1 for the terminator, and
** watching it score 7/7 on the cases above: ft_strlcat still produces the
** right text with only one spare byte to write the NUL into, and malloc's
** own rounding usually supplies that byte by accident. Only the allocated
** byte count, read from the wrapped allocator, tells the two apart.
*/

static void	join_case(t_ctx *c, const char *s1, const char *s2,
		const char *expected)
{
	char	*got;

	got = ft_strjoin(s1, s2);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (strcmp(got, expected))
		bro_fail(c->out, "expected \"%s\", got \"%s\"", expected, got);
	free(got);
}

static void	case_01(t_ctx *c) { join_case(c, "hello ", "world", "hello world"); }
static void	case_02(t_ctx *c) { join_case(c, "", "world", "world"); }
static void	case_03(t_ctx *c) { join_case(c, "hello", "", "hello"); }
static void	case_04(t_ctx *c) { join_case(c, "", "", ""); }

static void	case_05(t_ctx *c)
{
	char	a[101];
	char	b[101];
	char	want[201];

	memset(a, 'a', 100);
	a[100] = '\0';
	memset(b, 'b', 100);
	b[100] = '\0';
	memcpy(want, a, 100);
	memcpy(want + 100, b, 100);
	want[200] = '\0';
	join_case(c, a, b, want);
}

static void	case_06(t_ctx *c)
{
	char	s1[16];
	char	s2[16];
	char	*got;

	strcpy(s1, "hello ");
	strcpy(s2, "world");
	got = ft_strjoin(s1, s2);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (strcmp(s1, "hello ") || strcmp(s2, "world"))
		bro_fail(c->out, "s1 or s2 was modified");
	free(got);
}

static void	case_07(t_ctx *c)
{
	char	s1[16];
	char	s2[16];
	char	*got;

	strcpy(s1, "hello ");
	strcpy(s2, "world");
	got = ft_strjoin(s1, s2);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (got == s1 || got == s2)
		bro_fail(c->out, "result aliases an input pointer");
	free(got);
}

static void	case_08(t_ctx *c)
{
	t_alloc	before;
	t_alloc	after;
	char	*got;
	size_t	got_bytes;

	bro_alloc_snapshot(&before);
	got = ft_strjoin("ab", "cde");
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	bro_alloc_snapshot(&after);
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	got_bytes = after.live_bytes - before.live_bytes;
	if (got_bytes < 6)
		bro_fail(c->out, "the result needs at least 6 bytes, "
			"only %zu were allocated", got_bytes);
	free(got);
}

static const t_case	g_cases[] = {
{1, "ft_strjoin(\"hello \", \"world\")", BRO_INJECT, case_01},
{2, "ft_strjoin(\"\", \"world\")", BRO_INJECT, case_02},
{3, "ft_strjoin(\"hello\", \"\")", BRO_INJECT, case_03},
{4, "ft_strjoin(\"\", \"\")", BRO_INJECT, case_04},
{5, "ft_strjoin(<100 chars>, <100 chars>)", BRO_INJECT, case_05},
{6, "ft_strjoin(s1, s2): inputs unchanged", BRO_INJECT, case_06},
{7, "ft_strjoin(s1, s2): result is distinct", BRO_INJECT, case_07},
{8, "ft_strjoin(\"ab\", \"cde\"): exact allocation", BRO_INJECT, case_08},
};

const t_suite	g_suite_ft_strjoin = {
	"ft_strjoin", 2, 5, 2, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
