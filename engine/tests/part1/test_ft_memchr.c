#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_memchr - T0. Cases from _dev/SPEC_MICRO.md section 18.
** Oracle: memchr(s, c, n). Compared as an offset into the input.
*/

static void	chr_case(t_ctx *c, const char *s, int ch, size_t n)
{
	bro_expect_offset(c->out, s, memchr(s, ch, n), ft_memchr(s, ch, n));
}

static void	case_01(t_ctx *c) { chr_case(c, "abcdef", 'c', 6); }
static void	case_02(t_ctx *c) { chr_case(c, "abcdef", 'z', 6); }
static void	case_03(t_ctx *c) { chr_case(c, "abc\0def", 'd', 7); }
static void	case_04(t_ctx *c) { chr_case(c, "abc\0def", '\0', 7); }
static void	case_05(t_ctx *c) { chr_case(c, "abcdef", 'c', 2); }
static void	case_06(t_ctx *c) { chr_case(c, "abcdef", 'a', 0); }
static void	case_07(t_ctx *c) { chr_case(c, "\200\201\202", 0x81, 3); }
static void	case_08(t_ctx *c) { chr_case(c, "ABC", 0x141, 3); }

static const t_case	g_cases[] = {
{1, "ft_memchr(\"abcdef\", 'c', 6)", BRO_ORACLE, case_01},
{2, "ft_memchr(\"abcdef\", 'z', 6)", BRO_ORACLE, case_02},
{3, "ft_memchr(\"abc\\0def\", 'd', 7)", BRO_ORACLE, case_03},
{4, "ft_memchr(\"abc\\0def\", '\\0', 7)", BRO_ORACLE, case_04},
{5, "ft_memchr(\"abcdef\", 'c', 2)", BRO_ORACLE, case_05},
{6, "ft_memchr(\"abcdef\", 'a', 0)", BRO_ORACLE, case_06},
{7, "ft_memchr(\"\\200\\201\\202\", 0x81, 3)", BRO_ORACLE, case_07},
{8, "ft_memchr(\"ABC\", 0x141, 3)", BRO_ORACLE, case_08},
};

const t_suite	g_suite_ft_memchr = {
	"ft_memchr", 1, 2, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
