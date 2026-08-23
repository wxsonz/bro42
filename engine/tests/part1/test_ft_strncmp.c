#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_strncmp - T0. Cases from _dev/plan/rank00/libft-01-cases.md section 17.
** Oracle: strncmp(s1, s2, n), SIGN only.
**
** Only the sign is specified. Returning 1, -1 or the byte difference are all
** correct, and testing for an exact value is what makes other testers wrong.
*/

static void	cmp_case(t_ctx *c, const char *a, const char *b, size_t n)
{
	bro_expect_sign(c->out, strncmp(a, b, n), ft_strncmp(a, b, n));
}

static void	case_01(t_ctx *c) { cmp_case(c, "abcdef", "abcdef", 6); }
static void	case_02(t_ctx *c) { cmp_case(c, "abc", "abd", 3); }
static void	case_03(t_ctx *c) { cmp_case(c, "abd", "abc", 3); }
static void	case_04(t_ctx *c) { cmp_case(c, "abc", "abcdef", 6); }
static void	case_05(t_ctx *c) { cmp_case(c, "abc", "abd", 2); }
static void	case_06(t_ctx *c) { cmp_case(c, "abc", "xyz", 0); }
static void	case_07(t_ctx *c) { cmp_case(c, "test\200", "test\0", 6); }
static void	case_08(t_ctx *c) { cmp_case(c, "test\0", "test\200", 6); }

static const t_case	g_cases[] = {
{1, "ft_strncmp(\"abcdef\", \"abcdef\", 6)", BRO_ORACLE, case_01},
{2, "ft_strncmp(\"abc\", \"abd\", 3)", BRO_ORACLE, case_02},
{3, "ft_strncmp(\"abd\", \"abc\", 3)", BRO_ORACLE, case_03},
{4, "ft_strncmp(\"abc\", \"abcdef\", 6)", BRO_ORACLE, case_04},
{5, "ft_strncmp(\"abc\", \"abd\", 2)", BRO_ORACLE, case_05},
{6, "ft_strncmp(\"abc\", \"xyz\", 0)", BRO_ORACLE, case_06},
{7, "ft_strncmp(\"test\\200\", \"test\\0\", 6)", BRO_ORACLE, case_07},
{8, "ft_strncmp(\"test\\0\", \"test\\200\", 6)", BRO_ORACLE, case_08},
};

const t_suite	g_suite_ft_strncmp = {
	"ft_strncmp", 1, 3, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
