#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_bzero - T1. Cases transcribed from _dev/SPEC_MICRO.md section 8.
** Oracle: memset(s, 0, n), NOT bzero() - bzero was removed from POSIX in
** 2008 and is deprecated in glibc. The behaviour tested is identical.
*/

# define SCRATCH	16

static void	zero_case(t_ctx *c, size_t n)
{
	t_buf	*dst;
	t_buf	*ref;

	dst = bro_ctx_dst(c, SCRATCH);
	ref = bro_ctx_ref(c, SCRATCH);
	if (!dst || !ref)
		return (bro_fail(c->out, "engine: out of memory"));
	memset(ref->ptr, 0, n);
	ft_bzero(dst->ptr, n);
	bro_expect_bytes(c->out, ref->ptr, dst->ptr, SCRATCH);
}

static void	case_01(t_ctx *c) { zero_case(c, 5); }
static void	case_02(t_ctx *c) { zero_case(c, 10); }
static void	case_03(t_ctx *c) { zero_case(c, 0); }

/* Exact-width zero: the canary catches a loop that runs one step too far. */
static void	case_04(t_ctx *c)
{
	t_buf				*dst;
	static const unsigned char	want[4] = {0, 0, 0, 0};

	dst = bro_ctx_dst(c, 4);
	if (!dst)
		return (bro_fail(c->out, "engine: out of memory"));
	ft_bzero(dst->ptr, 4);
	bro_expect_bytes(c->out, want, dst->ptr, 4);
}

static const t_case	g_cases[] = {
{1, "ft_bzero(buf, 5)", BRO_ORACLE | BRO_GUARDED, case_01},
{2, "ft_bzero(buf, 10)", BRO_ORACLE | BRO_GUARDED, case_02},
{3, "ft_bzero(buf, 0)", BRO_ORACLE | BRO_GUARDED, case_03},
{4, "ft_bzero(buf4, 4)", BRO_GUARDED, case_04},
};

const t_suite	g_suite_ft_bzero = {
	"ft_bzero", 1, 2, 1, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
