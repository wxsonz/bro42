#include "bro.h"
#include "proto.h"
#include <ctype.h>

/*
** ft_isascii - T0. Cases from _dev/plan/rank00/libft-01-cases.md section 4.
** Oracle: isascii(c) != 0 ? 1 : 0
**
** The oracle is normalised to exactly 1 or 0 on purpose: libc promises only
** "non-zero", but Subject IV.2 asks for strictly 1 or 0, so comparing against
** the raw libc value would let a wrong-but-truthy return pass.
*/

static void	is_case(t_ctx *c, int arg)
{
	bro_expect_num(c->out, isascii(arg) != 0, ft_isascii(arg));
}

static void	case_01(t_ctx *c) { is_case(c, 0); }
static void	case_02(t_ctx *c) { is_case(c, 127); }
static void	case_03(t_ctx *c) { is_case(c, 'A'); }
static void	case_04(t_ctx *c) { is_case(c, -1); }
static void	case_05(t_ctx *c) { is_case(c, 128); }

/* The sweep. isascii is the one function in this family with no undefined
** region at all - isascii(c) is c >= 0 && c <= 127 for every int there is,
** not just EOF and unsigned char - so unlike ft_isalpha's pair (section 1)
** isascii is the one function in this family that is defined for EVERY int -
** it is simply c >= 0 && c <= 127 - so the notable-int row beside the grid is
** SCORED here, where for isalpha and friends it can only be shown. */
static void	case_06(t_ctx *c)
{
	bro_sweep_class(c->out, ft_isascii, isascii, 0, 127, 1);
}

static const long	g_ints[] = {-1, 128, 200, 255, 256, 1000};

static void	case_07(t_ctx *c)
{
	bro_sweep_ints(c->out, ft_isascii, isascii, g_ints,
			sizeof(g_ints) / sizeof(*g_ints), 1);
}

static const t_case	g_cases[] = {
{1, "ft_isascii(0)", BRO_ORACLE, case_01},
{2, "ft_isascii(127)", BRO_ORACLE, case_02},
{3, "ft_isascii('A')", BRO_ORACLE, case_03},
{4, "ft_isascii(-1)", BRO_ORACLE, case_04},
{5, "ft_isascii(128)", BRO_ORACLE, case_05},
{6, "ft_isascii(c) for every c in [0, 127]", BRO_ORACLE, case_06},
{7, "ft_isascii(c) for -1, 128, 200, 255, 256, 1000", BRO_ORACLE, case_07},
};

const t_suite	g_suite_ft_isascii = {
	"ft_isascii", 1, 1, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
