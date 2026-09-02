#include "bro.h"
#include "proto.h"
#include <string.h>

/*
** ft_strlcat - T1. Cases transcribed from _dev/plan/rank00/libft-01-cases.md section 12.
** Oracle: BSD strlcat(dst, src, size).
**
** Cases 5-8 are the branch everyone gets wrong: when size <= strlen(dst),
** nothing can be appended, dst is left alone, and the return is
** size + strlen(src) - NOT strlen(dst) + strlen(src), because strlcat has
** no way to know how long dst really is beyond the size it was given.
*/

static void	lcat_case(t_ctx *c, const char *dst0, const char *src,
		size_t size, size_t buflen)
{
	t_buf	*dst;
	t_buf	*ref;
	size_t	want;
	size_t	got;

	dst = bro_ctx_dst(c, buflen);
	ref = bro_ctx_ref(c, buflen);
	if (!dst || !ref)
		return (bro_fail(c->out, "engine: out of memory"));
	memcpy(dst->ptr, dst0, strlen(dst0) + 1);
	memcpy(ref->ptr, dst0, strlen(dst0) + 1);
	want = strlcat((char *)ref->ptr, src, size);
	got = ft_strlcat((char *)dst->ptr, src, size);
	bro_expect_bytes(c->out, ref->ptr, dst->ptr, buflen);
	bro_expect_num(c->out, (long long)want, (long long)got);
}

static void	case_01(t_ctx *c) { lcat_case(c, "hello", " world", 15, 16); }
static void	case_02(t_ctx *c) { lcat_case(c, "hello", " world", 8, 16); }
static void	case_03(t_ctx *c) { lcat_case(c, "", "42", 5, 16); }
static void	case_04(t_ctx *c) { lcat_case(c, "hello", "", 15, 16); }
static void	case_05(t_ctx *c) { lcat_case(c, "hello", " world", 5, 16); }
static void	case_06(t_ctx *c) { lcat_case(c, "hello", " world", 3, 16); }
static void	case_07(t_ctx *c) { lcat_case(c, "hello", " world", 0, 16); }
static void	case_08(t_ctx *c) { lcat_case(c, "hello world", "!", 4, 16); }
static void	case_09(t_ctx *c) { lcat_case(c, "hello", " world", 12, 12); }

static const t_case	g_cases[] = {
{1, "dst=\"hello\", ft_strlcat(dst, \" world\", 15)",
	BRO_ORACLE | BRO_GUARDED, case_01},
{2, "dst=\"hello\", ft_strlcat(dst, \" world\", 8)",
	BRO_ORACLE | BRO_GUARDED, case_02},
{3, "dst=\"\", ft_strlcat(dst, \"42\", 5)",
	BRO_ORACLE | BRO_GUARDED, case_03},
{4, "dst=\"hello\", ft_strlcat(dst, \"\", 15)",
	BRO_ORACLE | BRO_GUARDED, case_04},
{5, "dst=\"hello\", ft_strlcat(dst, \" world\", 5)",
	BRO_ORACLE | BRO_GUARDED, case_05},
{6, "dst=\"hello\", ft_strlcat(dst, \" world\", 3)",
	BRO_ORACLE | BRO_GUARDED, case_06},
{7, "dst=\"hello\", ft_strlcat(dst, \" world\", 0)",
	BRO_ORACLE | BRO_GUARDED, case_07},
{8, "dst=\"hello world\", ft_strlcat(dst, \"!\", 4)",
	BRO_ORACLE | BRO_GUARDED, case_08},
{9, "dst=\"hello\", ft_strlcat(dst12, \" world\", 12)",
	BRO_ORACLE | BRO_GUARDED, case_09},
};

const t_suite	g_suite_ft_strlcat = {
	"ft_strlcat", 1, 4, 1, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
