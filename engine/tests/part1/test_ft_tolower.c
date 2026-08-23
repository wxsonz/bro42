#include "bro.h"
#include "libft_proto.h"
#include <ctype.h>

/*
** ft_tolower - T0. Cases from _dev/plan/rank00/libft-01-cases.md section 14.
** Oracle: tolower(c)
**
** Unlike the ft_is* family this returns the character itself, so the raw libc
** value is the expectation - anything it does not convert must come back
** exactly as it went in.
*/

static void	to_case(t_ctx *c, int arg)
{
	bro_expect_num(c->out, tolower(arg), ft_tolower(arg));
}

static void	case_01(t_ctx *c) { to_case(c, 'A'); }
static void	case_02(t_ctx *c) { to_case(c, 'Z'); }
static void	case_03(t_ctx *c) { to_case(c, 'a'); }
static void	case_04(t_ctx *c) { to_case(c, '0'); }
static void	case_05(t_ctx *c) { to_case(c, '['); }
static void	case_06(t_ctx *c) { to_case(c, -1); }

/* The sweep. Mirror of ft_toupper's cases 07-08 - see that file for why this
** uses bro_sweep_map instead of bro_sweep_class. */
static void	case_07(t_ctx *c)
{
	bro_sweep_map(c->out, ft_tolower, tolower, -1, 255, 1);
}

static void	case_08(t_ctx *c)
{
	bro_sweep_map(c->out, ft_tolower, tolower, 256, 511, 0);
}

static const t_case	g_cases[] = {
{1, "ft_tolower('A')", BRO_ORACLE, case_01},
{2, "ft_tolower('Z')", BRO_ORACLE, case_02},
{3, "ft_tolower('a')", BRO_ORACLE, case_03},
{4, "ft_tolower('0')", BRO_ORACLE, case_04},
{5, "ft_tolower('[')", BRO_ORACLE, case_05},
{6, "ft_tolower(-1)", BRO_ORACLE, case_06},
{7, "ft_tolower(c) for every c in [-1, 255]", BRO_ORACLE, case_07},
{8, "ft_tolower(c) for every c in [256, 511]", BRO_UB_CASE, case_08},
};

const t_suite	g_suite_ft_tolower = {
	"ft_tolower", 1, 1, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
