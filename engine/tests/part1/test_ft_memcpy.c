#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_memcpy - T1. Cases transcribed from _dev/plan/rank00/libft-01-cases.md section 9.
** Oracle: memcpy(dest, src, n).
**
** Cases 6 and 7 are UB (the standard requires both pointers valid even at
** n = 0, and dest == src is an overlap memcpy never promises to handle) -
** they run and report what happened, but are never scored.
*/

# define SCRATCH	16

static void	copy_case(t_ctx *c, const char *src, size_t n)
{
	t_buf	*dst;
	t_buf	*ref;
	void	*ret;

	dst = bro_ctx_dst(c, SCRATCH);
	ref = bro_ctx_ref(c, SCRATCH);
	if (!dst || !ref)
		return (bro_fail(c->out, "engine: out of memory"));
	memcpy(ref->ptr, src, n);
	ret = ft_memcpy(dst->ptr, src, n);
	if (ret != dst->ptr)
		return (bro_fail(c->out, "must return the destination pointer"));
	bro_expect_bytes(c->out, ref->ptr, dst->ptr, SCRATCH);
}

static void	case_01(t_ctx *c) { copy_case(c, "hello", 6); }
static void	case_02(t_ctx *c) { copy_case(c, "ab\0cd\0f", 7); }
static void	case_03(t_ctx *c) { copy_case(c, "x", 1); }
static void	case_04(t_ctx *c) { copy_case(c, "hello", 0); }

/* Exact-width copy: the canary catches a loop that runs one step too far. */
static void	case_05(t_ctx *c)
{
	t_buf	*dst;

	dst = bro_ctx_dst(c, 4);
	if (!dst)
		return (bro_fail(c->out, "engine: out of memory"));
	ft_memcpy(dst->ptr, "abcd", 4);
	if (memcmp(dst->ptr, "abcd", 4) != 0)
		bro_fail(c->out, "the 4 copied bytes are wrong");
}

/*
** Undefined: the standard says nothing about NULL, NULL, 0. glibc returns
** dest without dereferencing either pointer; other libcs need not agree.
*/
/*
** glibc declares memcpy __nonnull, so passing a literal NULL to the LIBC one
** is rejected at compile time by -Werror=nonnull - which would cost this case
** its second data point. Going through a volatile function pointer defeats the
** static check without changing what runs, so the UB case can still show your
** behaviour beside glibc's, which is the whole point of the status (A5).
*/
static void	*(*volatile g_libc_memcpy)(void *, const void *, size_t) = memcpy;

static void	case_06(t_ctx *c)
{
	void	*got;
	void	*ref;

	got = ft_memcpy(NULL, NULL, 0);
	ref = g_libc_memcpy(NULL, NULL, 0);
	bro_mark_ub(c->out,
		"yours returns %s, glibc returns %s - the standard leaves memcpy "
		"with null pointers undefined even when n is 0",
		got ? "non-NULL" : "NULL", ref ? "non-NULL" : "NULL");
}

/*
** Undefined: dest == src is a full overlap memcpy is not required to handle,
** even though it happens to be harmless here (every byte is copied onto
** itself). There is no defined answer to compare against.
*/
static void	case_07(t_ctx *c)
{
	t_buf	*dst;
	void	*got;

	dst = bro_ctx_dst(c, 5);
	if (!dst)
		return (bro_fail(c->out, "engine: out of memory"));
	memcpy(dst->ptr, "hello", 5);
	got = ft_memcpy(dst->ptr, dst->ptr, 5);
	bro_mark_ub(c->out, "yours returns dest%+ld - dest == src is overlap",
		(long)((unsigned char *)got - dst->ptr));
}

/*
** The same overlap as ft_memmove case 3 - deliberately the same buffer, the
** same offsets, the same length - so the two reports can be read side by side.
** memcpy is ALLOWED to assume its regions do not overlap and real
** implementations exploit that to copy in whatever order is fastest, so there
** is no correct answer here to grade against. Shown, never scored: what the
** student needs to see is that the two functions answer this differently, and
** that is the first question the defense bank asks.
*/
static void	case_08(t_ctx *c)
{
	t_buf	*dst;
	char	want[10];
	char	got[10];

	dst = bro_ctx_dst(c, SCRATCH);
	if (!dst)
		return (bro_fail(c->out, "engine: out of memory"));
	memcpy(dst->ptr, "abcdefgh", 9);
	memcpy(want, "abcdefgh", 9);
	memmove(want + 2, want, 5);
	ft_memcpy((char *)dst->ptr + 2, dst->ptr, 5);
	memcpy(got, dst->ptr, 9);
	got[9] = '\0';
	want[9] = '\0';
	bro_mark_ub(c->out, "overlapping by 3 bytes: yours gives \"%s\", "
		"ft_memmove is required to give \"%s\" - memcpy never promises "
		"to handle overlap, and that is the whole reason memmove exists",
		got, want);
}

static const t_case	g_cases[] = {
{1, "ft_memcpy(dest, \"hello\", 6)", BRO_ORACLE | BRO_GUARDED, case_01},
{2, "ft_memcpy(dest, \"ab\\0cd\\0f\", 7)", BRO_ORACLE | BRO_GUARDED, case_02},
{3, "ft_memcpy(dest, \"x\", 1)", BRO_ORACLE | BRO_GUARDED, case_03},
{4, "ft_memcpy(dest, \"hello\", 0)", BRO_ORACLE | BRO_GUARDED, case_04},
{5, "ft_memcpy(dest4, \"abcd\", 4)", BRO_GUARDED, case_05},
{6, "ft_memcpy(NULL, NULL, 0)", BRO_UB_CASE, case_06},
{7, "ft_memcpy(buf, buf, 5)", BRO_UB_CASE, case_07},
{8, "ft_memcpy(buf + 2, buf, 5) - overlapping", BRO_UB_CASE, case_08},
};

const t_suite	g_suite_ft_memcpy = {
	"ft_memcpy", 1, 3, 1, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
