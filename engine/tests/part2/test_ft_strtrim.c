#include "bro.h"
#include "libft_proto.h"
#include <stdlib.h>
#include <string.h>

/*
** ft_strtrim - T2. Cases from _dev/SPEC_MICRO.md section 26.
**
** Cases 8 and 9 pass an empty set. A set-membership check that never
** terminates for an empty set is the classic Libft infinite loop; the
** engine's own timeout turns that into a TIMEOUT verdict rather than a hang,
** which is correct behaviour to report, not something to work around here.
*/

static void	trim_case(t_ctx *c, const char *s, const char *set,
		const char *expected)
{
	char	*got;

	got = ft_strtrim(s, set);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (strcmp(got, expected))
		bro_fail(c->out, "expected \"%s\", got \"%s\"", expected, got);
	free(got);
}

static void	case_01(t_ctx *c)
{
	trim_case(c, "   hello world   ", " ", "hello world");
}
static void	case_02(t_ctx *c)
{
	trim_case(c, "xxxyyyhello worldyyyxxx", "xy", "hello world");
}
static void	case_03(t_ctx *c) { trim_case(c, "aabaa", "a", "b"); }
static void	case_04(t_ctx *c) { trim_case(c, "hello", "xyz", "hello"); }
static void	case_05(t_ctx *c) { trim_case(c, "xxxxxx", "x", ""); }
static void	case_06(t_ctx *c) { trim_case(c, " ", " ", ""); }
static void	case_07(t_ctx *c) { trim_case(c, "", "abc", ""); }
static void	case_08(t_ctx *c) { trim_case(c, "hello", "", "hello"); }
static void	case_09(t_ctx *c) { trim_case(c, "", "", ""); }

static const t_case	g_cases[] = {
{1, "ft_strtrim(\"   hello world   \", \" \")", BRO_INJECT, case_01},
{2, "ft_strtrim(\"xxxyyyhello worldyyyxxx\", \"xy\")", BRO_INJECT, case_02},
{3, "ft_strtrim(\"aabaa\", \"a\")", BRO_INJECT, case_03},
{4, "ft_strtrim(\"hello\", \"xyz\")", BRO_INJECT, case_04},
{5, "ft_strtrim(\"xxxxxx\", \"x\")", BRO_INJECT, case_05},
{6, "ft_strtrim(\" \", \" \")", BRO_INJECT, case_06},
{7, "ft_strtrim(\"\", \"abc\")", BRO_INJECT, case_07},
{8, "ft_strtrim(\"hello\", \"\")", BRO_INJECT, case_08},
{9, "ft_strtrim(\"\", \"\")", BRO_INJECT, case_09},
};

const t_suite	g_suite_ft_strtrim = {
	"ft_strtrim", 2, 5, 2, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
