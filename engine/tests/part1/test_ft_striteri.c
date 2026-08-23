#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_striteri - T1. Cases transcribed from _dev/plan/rank00/libft-01-cases.md section 30.
**
** No oracle: striteri has no libc equivalent, so cases check the result
** directly. It writes into a caller-owned buffer through the pointer it is
** handed to f, so the destination is still GUARDED.
**
** Case 6 hands it a string literal, which lives in read-only memory - it
** segfaults inside ft_striteri on every platform 42 uses. That crash is
** expected: the case is flagged UB, so the engine reports it as UB rather
** than as a failure (design/04_TESTDESIGN.md A5).
*/

static int	g_add_calls;

static void	add_index(unsigned int i, char *c)
{
	g_add_calls++;
	*c = (char)(*c + i);
}

static void	to_x(unsigned int i, char *c)
{
	(void)i;
	*c = 'X';
}

static unsigned int	g_track[8];
static size_t		g_track_n;

static void	record_index(unsigned int i, char *c)
{
	(void)c;
	if (g_track_n < 8)
		g_track[g_track_n++] = i;
}

static void	case_01(t_ctx *c)
{
	t_buf				*dst;
	static const unsigned char	want[4] = "ace";

	dst = bro_ctx_dst(c, 4);
	if (!dst)
		return (bro_fail(c->out, "engine: out of memory"));
	memcpy(dst->ptr, "abc", 4);
	ft_striteri((char *)dst->ptr, add_index);
	bro_expect_bytes(c->out, want, dst->ptr, 4);
}

static void	case_02(t_ctx *c)
{
	t_buf	*dst;

	dst = bro_ctx_dst(c, 1);
	if (!dst)
		return (bro_fail(c->out, "engine: out of memory"));
	dst->ptr[0] = '\0';
	g_add_calls = 0;
	ft_striteri((char *)dst->ptr, add_index);
	if (g_add_calls != 0)
		return (bro_fail(c->out, "f was called %d time(s) on an empty string",
				g_add_calls));
	if (dst->ptr[0] != '\0')
		bro_fail(c->out, "the empty string must stay empty");
}

static void	case_03(t_ctx *c)
{
	t_buf				*dst;
	static const unsigned char	want[4] = "XXX";

	dst = bro_ctx_dst(c, 4);
	if (!dst)
		return (bro_fail(c->out, "engine: out of memory"));
	memcpy(dst->ptr, "abc", 4);
	ft_striteri((char *)dst->ptr, to_x);
	bro_expect_bytes(c->out, want, dst->ptr, 4);
}

static void	case_04(t_ctx *c)
{
	t_buf	*dst;
	size_t	i;

	dst = bro_ctx_dst(c, 6);
	if (!dst)
		return (bro_fail(c->out, "engine: out of memory"));
	memcpy(dst->ptr, "abcde", 6);
	g_track_n = 0;
	ft_striteri((char *)dst->ptr, record_index);
	if (g_track_n != 5)
		return (bro_fail(c->out, "f was called %zu time(s), expected 5",
				g_track_n));
	i = 0;
	while (i < 5)
	{
		if (g_track[i] != i)
			return (bro_fail(c->out, "index %zu: expected %zu, saw %u",
					i, i, g_track[i]));
		i++;
	}
}

/* striteri returns void and owns no memory - any allocation is a leak. */
static void	case_05(t_ctx *c)
{
	t_buf	*dst;
	t_alloc	before;
	t_alloc	after;

	dst = bro_ctx_dst(c, 4);
	if (!dst)
		return (bro_fail(c->out, "engine: out of memory"));
	memcpy(dst->ptr, "abc", 4);
	bro_ready(c);
	bro_alloc_snapshot(&before);
	ft_striteri((char *)dst->ptr, add_index);
	bro_alloc_snapshot(&after);
	if (after.calls != before.calls)
		bro_fail(c->out, "striteri made %zu malloc call(s) - it owns no memory",
			(size_t)(after.calls - before.calls));
}

/*
** Undefined: a string literal is read-only, so writing through the pointer
** f is given faults. Nothing is required of the implementation here - just
** that it does not silently corrupt something else instead.
*/
static void	case_06(t_ctx *c)
{
	ft_striteri("hello", add_index);
	bro_mark_ub(c->out, "did not crash - a string literal is read-only memory");
}

static const t_case	g_cases[] = {
{1, "ft_striteri(\"abc\", f) with f: *c += i", BRO_GUARDED, case_01},
{2, "ft_striteri(\"\", f) with f: *c += i", BRO_GUARDED, case_02},
{3, "ft_striteri(\"abc\", f) with f: *c = 'X'", BRO_GUARDED, case_03},
{4, "ft_striteri(\"abcde\", f) recording indices", BRO_GUARDED, case_04},
{5, "ft_striteri(\"abc\", f): zero bytes allocated", BRO_INJECT | BRO_GUARDED,
	case_05},
{6, "ft_striteri(<string literal>, f)", BRO_UB_CASE, case_06},
};

const t_suite	g_suite_ft_striteri = {
	"ft_striteri", 2, 6, 1, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
