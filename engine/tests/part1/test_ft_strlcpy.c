#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_strlcpy - T1. Cases transcribed from _dev/plan/rank00/libft-01-cases.md section 11.
** Oracle: bro_ref_strlcpy - BSD strlcpy is not on every glibc (only since
** 2.38), so the engine ships its own (engine/src/oracle.c).
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
	want = bro_ref_strlcpy((char *)ref->ptr, src, size);
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
** size is 0. bro_ref_strlcpy happens not to dereference dst in that case,
** but nothing in the BSD docs promises that of any implementation.
*/
static void	case_08(t_ctx *c)
{
	size_t	got;
	size_t	ref;

	got = ft_strlcpy(NULL, "hello", 0);
	ref = bro_ref_strlcpy(NULL, "hello", 0);
	bro_mark_ub(c->out,
		"yours returns %zu, the reference returns %zu - a NULL "
		"destination is undefined by BSD even at size 0", got, ref);
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
