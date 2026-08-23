#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_strlcpy - T1. Cases transcribed from _dev/SPEC_MICRO.md section 11.
** Oracle: BSD strlcpy(dst, src, size) - glibc has carried it natively
** since 2.38.
**
** Case 8 is UB (NULL destination, even at size 0 - BSD never defined it).
*/

static void	lcpy_case(t_ctx *c, const char *src, size_t size, size_t buflen)
{
	t_buf	*dst;
	t_buf	*ref;
	size_t	want;
	size_t	got;

	dst = bro_ctx_dst(c, buflen);
	ref = bro_ctx_ref(c, buflen);
	if (!dst || !ref)
		return (bro_fail(c->out, "engine: out of memory"));
	want = strlcpy((char *)ref->ptr, src, size);
	got = ft_strlcpy((char *)dst->ptr, src, size);
	bro_expect_bytes(c->out, ref->ptr, dst->ptr, buflen);
	bro_expect_num(c->out, (long long)want, (long long)got);
}

static void	case_01(t_ctx *c) { lcpy_case(c, "hello", 10, 16); }
static void	case_02(t_ctx *c) { lcpy_case(c, "hello", 3, 16); }
static void	case_03(t_ctx *c) { lcpy_case(c, "hello", 1, 16); }
static void	case_04(t_ctx *c) { lcpy_case(c, "", 5, 16); }
static void	case_05(t_ctx *c) { lcpy_case(c, "hello", 0, 16); }
static void	case_06(t_ctx *c) { lcpy_case(c, "hello", 6, 6); }
static void	case_07(t_ctx *c) { lcpy_case(c, "hello!", 6, 6); }

/*
** Undefined: BSD never defines strlcpy with a NULL destination, even when
** size is 0. glibc 2.38+ happens not to dereference it here, but nothing
** promises that.
*/
/*
** As in test_ft_memcpy.c: glibc declares strlcpy __nonnull, so a literal NULL
** is a compile-time error under -Werror. The volatile function pointer keeps
** the glibc half of the comparison, which is what makes a UB case worth
** running at all.
*/
static size_t	(*volatile g_libc_strlcpy)(char *, const char *, size_t)
	= strlcpy;

static void	case_08(t_ctx *c)
{
	size_t	got;
	size_t	ref;

	got = ft_strlcpy(NULL, "hello", 0);
	ref = g_libc_strlcpy(NULL, "hello", 0);
	bro_mark_ub(c->out,
		"yours returns %zu, glibc returns %zu - a NULL destination is "
		"undefined by BSD even at size 0", got, ref);
}

static const t_case	g_cases[] = {
{1, "ft_strlcpy(dst, \"hello\", 10)", BRO_ORACLE | BRO_GUARDED, case_01},
{2, "ft_strlcpy(dst, \"hello\", 3)", BRO_ORACLE | BRO_GUARDED, case_02},
{3, "ft_strlcpy(dst, \"hello\", 1)", BRO_ORACLE | BRO_GUARDED, case_03},
{4, "ft_strlcpy(dst, \"\", 5)", BRO_ORACLE | BRO_GUARDED, case_04},
{5, "ft_strlcpy(dst, \"hello\", 0)", BRO_ORACLE | BRO_GUARDED, case_05},
{6, "ft_strlcpy(dst6, \"hello\", 6)", BRO_ORACLE | BRO_GUARDED, case_06},
{7, "ft_strlcpy(dst6, \"hello!\", 6)", BRO_ORACLE | BRO_GUARDED, case_07},
{8, "ft_strlcpy(NULL, \"hello\", 0)", BRO_UB_CASE, case_08},
};

const t_suite	g_suite_ft_strlcpy = {
	"ft_strlcpy", 1, 4, 1, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
