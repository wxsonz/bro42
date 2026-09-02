#include "bro.h"
#include "proto.h"

/*
** ft_strnstr - T0. Cases from _dev/plan/rank00/libft-01-cases.md section 20.
** Oracle: bro_ref_strnstr - strnstr is BSD-only and glibc will never have it,
** so the engine ships its own (engine/core/oracle.c).
*/

static void	nstr_case(t_ctx *c, const char *big, const char *little, size_t n)
{
	bro_expect_offset(c->out, big, bro_ref_strnstr(big, little, n),
		ft_strnstr(big, little, n));
}

static void	case_01(t_ctx *c) { nstr_case(c, "Hello World", "World", 11); }
static void	case_02(t_ctx *c) { nstr_case(c, "Hello World", "World", 10); }
static void	case_03(t_ctx *c) { nstr_case(c, "Hello World", "xyz", 11); }
static void	case_04(t_ctx *c) { nstr_case(c, "Hello", "Hello", 5); }
static void	case_05(t_ctx *c) { nstr_case(c, "aaabc", "aabc", 5); }
static void	case_06(t_ctx *c) { nstr_case(c, "aaa", "aab", 3); }
static void	case_07(t_ctx *c) { nstr_case(c, "Hello World", "", 5); }
static void	case_08(t_ctx *c) { nstr_case(c, "Hello World", "", 0); }
static void	case_09(t_ctx *c) { nstr_case(c, "abc", "c", 100); }

static const t_case	g_cases[] = {
{1, "ft_strnstr(\"Hello World\", \"World\", 11)", BRO_ORACLE, case_01},
{2, "ft_strnstr(\"Hello World\", \"World\", 10)", BRO_ORACLE, case_02},
{3, "ft_strnstr(\"Hello World\", \"xyz\", 11)", BRO_ORACLE, case_03},
{4, "ft_strnstr(\"Hello\", \"Hello\", 5)", BRO_ORACLE, case_04},
{5, "ft_strnstr(\"aaabc\", \"aabc\", 5)", BRO_ORACLE, case_05},
{6, "ft_strnstr(\"aaa\", \"aab\", 3)", BRO_ORACLE, case_06},
{7, "ft_strnstr(\"Hello World\", \"\", 5)", BRO_ORACLE, case_07},
{8, "ft_strnstr(\"Hello World\", \"\", 0)", BRO_ORACLE, case_08},
{9, "ft_strnstr(\"abc\", \"c\", 100)", BRO_ORACLE, case_09},
};

const t_suite	g_suite_ft_strnstr = {
	"ft_strnstr", 1, 4, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
