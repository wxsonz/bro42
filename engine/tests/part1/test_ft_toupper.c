#include "bro.h"
#include "libft_proto.h"
#include <ctype.h>

/*
** ft_toupper - T0. Cases from _dev/plan/rank00/libft-01-cases.md section 13.
** Oracle: toupper(c)
**
** Unlike the ft_is* family this returns the character itself, so the raw libc
** value is the expectation - anything it does not convert must come back
** exactly as it went in.
*/

static void	to_case(t_ctx *c, int arg)
{
	bro_expect_num(c->out, toupper(arg), ft_toupper(arg));
}

static void	case_01(t_ctx *c) { to_case(c, 'a'); }
static void	case_02(t_ctx *c) { to_case(c, 'z'); }
static void	case_03(t_ctx *c) { to_case(c, 'A'); }
static void	case_04(t_ctx *c) { to_case(c, '0'); }
static void	case_05(t_ctx *c) { to_case(c, '{'); }
static void	case_06(t_ctx *c) { to_case(c, -1); }

static const t_case	g_cases[] = {
{1, "ft_toupper('a')", BRO_ORACLE, case_01},
{2, "ft_toupper('z')", BRO_ORACLE, case_02},
{3, "ft_toupper('A')", BRO_ORACLE, case_03},
{4, "ft_toupper('0')", BRO_ORACLE, case_04},
{5, "ft_toupper('{')", BRO_ORACLE, case_05},
{6, "ft_toupper(-1)", BRO_ORACLE, case_06},
};

const t_suite	g_suite_ft_toupper = {
	"ft_toupper", 1, 1, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
