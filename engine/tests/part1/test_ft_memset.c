#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_memset - T1. Cases transcribed from _dev/plan/rank00/libft-01-cases.md section 7.
** Oracle: memset(s, c, n).
**
** Every case runs against a GUARDED destination, and the oracle writes into
** its own separate buffer (ctx->ref). Sharing one buffer between libc and the
** student would compare a buffer against itself and pass every case.
*/

# define SCRATCH	16

static void	fill_case(t_ctx *c, int ch, size_t n)
{
	t_buf	*dst;
	t_buf	*ref;
	void	*ret;

	dst = bro_ctx_dst(c, SCRATCH);
	ref = bro_ctx_ref(c, SCRATCH);
	if (!dst || !ref)
		return (bro_fail(c->out, "engine: out of memory"));
	memset(ref->ptr, ch, n);
	ret = ft_memset(dst->ptr, ch, n);
	if (ret != dst->ptr)
		return (bro_fail(c->out, "must return the original pointer"));
	bro_expect_bytes(c->out, ref->ptr, dst->ptr, SCRATCH);
}

static void	case_01(t_ctx *c) { fill_case(c, 'A', 5); }
static void	case_02(t_ctx *c) { fill_case(c, 0, 10); }
static void	case_03(t_ctx *c) { fill_case(c, 'A', 1); }
static void	case_04(t_ctx *c) { fill_case(c, 'X', 0); }
static void	case_05(t_ctx *c) { fill_case(c, 0x123441, 5); }
static void	case_06(t_ctx *c) { fill_case(c, -1, 5); }

/* Exact-width fill: the canary catches a loop that runs one step too far. */
static void	case_07(t_ctx *c)
{
	t_buf				*dst;
	static const unsigned char	want[4] = {'A', 'A', 'A', 'A'};

	dst = bro_ctx_dst(c, 4);
	if (!dst)
		return (bro_fail(c->out, "engine: out of memory"));
	ft_memset(dst->ptr, 'A', 4);
	bro_expect_bytes(c->out, want, dst->ptr, 4);
}

static const t_case	g_cases[] = {
{1, "ft_memset(buf, 'A', 5)", BRO_ORACLE | BRO_GUARDED, case_01},
{2, "ft_memset(buf, 0, 10)", BRO_ORACLE | BRO_GUARDED, case_02},
{3, "ft_memset(buf, 'A', 1)", BRO_ORACLE | BRO_GUARDED, case_03},
{4, "ft_memset(buf, 'X', 0)", BRO_ORACLE | BRO_GUARDED, case_04},
{5, "ft_memset(buf, 0x123441, 5)", BRO_ORACLE | BRO_GUARDED, case_05},
{6, "ft_memset(buf, -1, 5)", BRO_ORACLE | BRO_GUARDED, case_06},
{7, "ft_memset(buf4, 'A', 4)", BRO_GUARDED, case_07},
};

const t_suite	g_suite_ft_memset = {
	"ft_memset", 1, 2, 1, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
