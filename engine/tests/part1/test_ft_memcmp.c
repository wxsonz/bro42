#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_memcmp - T0. Cases transcribed from _dev/plan/rank00/libft-01-cases.md section 19.
** Oracle: memcmp(s1, s2, n), sign equality only.
**
** IDs are permanent addresses (design/04 B7). Append only; never renumber.
*/

/*
** memcmp only reads, so the inputs need no guarded scratch - literals are fine.
** Only the sign is compared: the standard specifies nothing more, and testing
** for an exact 1 or -1 is what makes other testers wrong.
*/
static void	cmp_case(t_ctx *c, const char *a, const char *b, size_t n)
{
	int	expected;
	int	actual;

	expected = memcmp(a, b, n);
	actual = ft_memcmp(a, b, n);
	bro_expect_sign(c->out, expected, actual);
}

static void	case_01(t_ctx *c) { cmp_case(c, "abcdef", "abcdef", 6); }
static void	case_02(t_ctx *c) { cmp_case(c, "abc\0def", "abc\0deg", 7); }
static void	case_03(t_ctx *c) { cmp_case(c, "abc", "xyz", 0); }
static void	case_04(t_ctx *c) { cmp_case(c, "\200", "\0", 1); }
static void	case_05(t_ctx *c) { cmp_case(c, "\0", "\200", 1); }
static void	case_06(t_ctx *c) { cmp_case(c, "\377\0", "\0\0", 2); }

static const t_case	g_cases[] = {
{1, "ft_memcmp(\"abcdef\", \"abcdef\", 6)", BRO_ORACLE, case_01},
{2, "ft_memcmp(\"abc\\0def\", \"abc\\0deg\", 7)", BRO_ORACLE, case_02},
{3, "ft_memcmp(\"abc\", \"xyz\", 0)", BRO_ORACLE, case_03},
{4, "ft_memcmp(\"\\200\", \"\\0\", 1)", BRO_ORACLE, case_04},
{5, "ft_memcmp(\"\\0\", \"\\200\", 1)", BRO_ORACLE, case_05},
{6, "ft_memcmp(\"\\377\\0\", \"\\0\\0\", 2)", BRO_ORACLE, case_06},
};

const t_suite	g_suite_ft_memcmp = {
	"ft_memcmp", 1, 3, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
