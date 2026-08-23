#include "bro.h"
#include "libft_proto.h"
#include <ctype.h>

/*
** ft_isalnum - T0. Cases from _dev/plan/rank00/libft-01-cases.md section 3.
** Oracle: isalnum(c) != 0 ? 1 : 0
**
** The oracle is normalised to exactly 1 or 0 on purpose: libc promises only
** "non-zero", but Subject IV.2 asks for strictly 1 or 0, so comparing against
** the raw libc value would let a wrong-but-truthy return pass.
*/

static void	is_case(t_ctx *c, int arg)
{
	bro_expect_num(c->out, isalnum(arg) != 0, ft_isalnum(arg));
}

static void	case_01(t_ctx *c) { is_case(c, 'a'); }
static void	case_02(t_ctx *c) { is_case(c, 'Z'); }
static void	case_03(t_ctx *c) { is_case(c, '5'); }
static void	case_04(t_ctx *c) { is_case(c, ' '); }
static void	case_05(t_ctx *c) { is_case(c, '@'); }
static void	case_06(t_ctx *c) { is_case(c, '['); }
static void	case_07(t_ctx *c) { is_case(c, -1); }

/* The sweep. Same shape as ft_isalpha's cases 10-11 (section 1). Note that
** the union property (isalnum = isalpha || isdigit) means this sweep also
** re-proves both of those ranges at once. */
static void	case_08(t_ctx *c)
{
	bro_sweep_class(c->out, ft_isalnum, isalnum, -1, 255, 1);
}

static void	case_09(t_ctx *c)
{
	bro_sweep_class(c->out, ft_isalnum, isalnum, 256, 511, 0);
}

static const t_case	g_cases[] = {
{1, "ft_isalnum('a')", BRO_ORACLE, case_01},
{2, "ft_isalnum('Z')", BRO_ORACLE, case_02},
{3, "ft_isalnum('5')", BRO_ORACLE, case_03},
{4, "ft_isalnum(' ')", BRO_ORACLE, case_04},
{5, "ft_isalnum('@')", BRO_ORACLE, case_05},
{6, "ft_isalnum('[')", BRO_ORACLE, case_06},
{7, "ft_isalnum(-1)", BRO_ORACLE, case_07},
{8, "ft_isalnum(c) for every c in [-1, 255]", BRO_ORACLE, case_08},
{9, "ft_isalnum(c) for every c in [256, 511]", BRO_UB_CASE, case_09},
};

const t_suite	g_suite_ft_isalnum = {
	"ft_isalnum", 1, 1, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
