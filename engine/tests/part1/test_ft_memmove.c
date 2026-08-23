#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_memmove - T1. Cases transcribed from _dev/SPEC_MICRO.md section 10.
** Oracle: memmove(dest, src, n).
**
** Every case runs against a GUARDED destination, and the oracle writes into
** its own separate buffer (ctx->ref). Sharing one buffer between libc and the
** student would compare a buffer against itself and pass every case.
*/

# define SCRATCH	16

/* Non-overlapping: dest and src are distinct buffers. */
static void	copy_case(t_ctx *c, const char *src, size_t n)
{
	t_buf	*dst;
	t_buf	*ref;
	void	*ret;

	dst = bro_ctx_dst(c, SCRATCH);
	ref = bro_ctx_ref(c, SCRATCH);
	if (!dst || !ref)
		return (bro_fail(c->out, "engine: out of memory"));
	memmove(ref->ptr, src, n);
	ret = ft_memmove(dst->ptr, src, n);
	if (ret != dst->ptr)
		return (bro_fail(c->out, "must return the destination pointer"));
	bro_expect_bytes(c->out, ref->ptr, dst->ptr, SCRATCH);
}

/*
** Overlapping: dest and src are offsets into ONE buffer. This is the case the
** function exists for - a forward copy destroys bytes it has not read yet.
*/
static void	overlap_case(t_ctx *c, long doff, long soff, size_t n)
{
	t_buf	*dst;
	t_buf	*ref;
	void	*ret;

	dst = bro_ctx_dst(c, SCRATCH);
	ref = bro_ctx_ref(c, SCRATCH);
	if (!dst || !ref)
		return (bro_fail(c->out, "engine: out of memory"));
	memcpy(dst->ptr, "abcde", 6);
	memcpy(ref->ptr, "abcde", 6);
	memmove(ref->ptr + doff, ref->ptr + soff, n);
	ret = ft_memmove(dst->ptr + doff, dst->ptr + soff, n);
	if (ret != dst->ptr + doff)
		return (bro_fail(c->out, "must return the destination pointer"));
	bro_expect_bytes(c->out, ref->ptr, dst->ptr, SCRATCH);
}

static void	case_01(t_ctx *c) { copy_case(c, "hello", 6); }
static void	case_02(t_ctx *c) { copy_case(c, "ab\0cd\0f", 7); }
static void	case_03(t_ctx *c) { overlap_case(c, 2, 0, 5); }
static void	case_04(t_ctx *c) { overlap_case(c, 1, 0, 5); }
static void	case_05(t_ctx *c) { overlap_case(c, 0, 2, 5); }
static void	case_06(t_ctx *c) { overlap_case(c, 0, 0, 5); }
static void	case_07(t_ctx *c) { overlap_case(c, 0, 1, 0); }

/* Exact-width copy: the canary catches a loop that runs one step too far. */
static void	case_08(t_ctx *c)
{
	t_buf	*dst;

	dst = bro_ctx_dst(c, 4);
	if (!dst)
		return (bro_fail(c->out, "engine: out of memory"));
	ft_memmove(dst->ptr, "abcd", 4);
	if (memcmp(dst->ptr, "abcd", 4) != 0)
		bro_fail(c->out, "the 4 copied bytes are wrong");
}

static const t_case	g_cases[] = {
{1, "ft_memmove(dest, \"hello\", 6)", BRO_ORACLE | BRO_GUARDED, case_01},
{2, "ft_memmove(dest, \"ab\\0cd\\0f\", 7)", BRO_ORACLE | BRO_GUARDED, case_02},
{3, "ft_memmove(buf + 2, buf, 5)", BRO_ORACLE | BRO_GUARDED, case_03},
{4, "ft_memmove(buf + 1, buf, 5)", BRO_ORACLE | BRO_GUARDED, case_04},
{5, "ft_memmove(buf, buf + 2, 5)", BRO_ORACLE | BRO_GUARDED, case_05},
{6, "ft_memmove(buf, buf, 5)", BRO_ORACLE | BRO_GUARDED, case_06},
{7, "ft_memmove(buf, buf + 1, 0)", BRO_ORACLE | BRO_GUARDED, case_07},
{8, "ft_memmove(dest4, \"abcd\", 4)", BRO_GUARDED, case_08},
};

const t_suite	g_suite_ft_memmove = {
	"ft_memmove", 1, 3, 1, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
