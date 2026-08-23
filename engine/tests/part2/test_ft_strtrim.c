#include "bro.h"
#include "libft_proto.h"
#include <stdlib.h>
#include <string.h>

/*
** ft_strtrim - T2. Cases from _dev/plan/rank00/libft-01-cases.md section 26.
**
** Cases 2, 10, 11 and 12 trim a DIFFERENT number of bytes off each end, and
** case 2's runs interleave the set rather than blocking it. Before that, every
** trimming case was symmetric - 3/3, 6/6, 2/2 - so an implementation that
** measured the front and mirrored the same count off the back passed all nine.
** That was verified by writing one and watching it score 9/9.
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
	trim_case(c, "yxxyyxhello worldyyxyx", "xy", "hello world");
}
static void	case_03(t_ctx *c) { trim_case(c, "aabaa", "a", "b"); }
static void	case_04(t_ctx *c) { trim_case(c, "hello", "xyz", "hello"); }
static void	case_05(t_ctx *c) { trim_case(c, "xxxxxx", "x", ""); }
static void	case_06(t_ctx *c) { trim_case(c, " ", " ", ""); }
static void	case_07(t_ctx *c) { trim_case(c, "", "abc", ""); }
static void	case_08(t_ctx *c) { trim_case(c, "hello", "", "hello"); }
static void	case_09(t_ctx *c) { trim_case(c, "", "", ""); }
static void	case_10(t_ctx *c) { trim_case(c, "xxxhello", "xy", "hello"); }
static void	case_11(t_ctx *c) { trim_case(c, "helloyyyy", "xy", "hello"); }
static void	case_12(t_ctx *c)
{
	trim_case(c, "xhelloyyyyyy", "xy", "hello");
}

static const t_case	g_cases[] = {
{1, "ft_strtrim(\"   hello world   \", \" \")", BRO_INJECT, case_01},
{2, "ft_strtrim(\"yxxyyxhello worldyyxyx\", \"xy\")", BRO_INJECT, case_02},
{3, "ft_strtrim(\"aabaa\", \"a\")", BRO_INJECT, case_03},
{4, "ft_strtrim(\"hello\", \"xyz\")", BRO_INJECT, case_04},
{5, "ft_strtrim(\"xxxxxx\", \"x\")", BRO_INJECT, case_05},
{6, "ft_strtrim(\" \", \" \")", BRO_INJECT, case_06},
{7, "ft_strtrim(\"\", \"abc\")", BRO_INJECT, case_07},
{8, "ft_strtrim(\"hello\", \"\")", BRO_INJECT, case_08},
{9, "ft_strtrim(\"\", \"\")", BRO_INJECT, case_09},
{10, "ft_strtrim(\"xxxhello\", \"xy\")", BRO_INJECT, case_10},
{11, "ft_strtrim(\"helloyyyy\", \"xy\")", BRO_INJECT, case_11},
{12, "ft_strtrim(\"xhelloyyyyyy\", \"xy\")", BRO_INJECT, case_12},
};

const t_suite	g_suite_ft_strtrim = {
	"ft_strtrim", 2, 5, 2, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
